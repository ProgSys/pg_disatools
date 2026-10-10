#include "PG_DisaColorReducer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace {
	/// Above this many distinct colors the clustering works on colors rounded to 5 bits, the mapping stays exact
	constexpr std::size_t g_maxClusterPoints = 32768;
	constexpr int g_kMeansPasses = 6;

	struct ColorPoint {
		glm::vec4 position; /// OKLab with alpha as fourth axis, all roughly in [0, 1]
		glm::vec4 color; /// rgba in [0, 255], a mean when points were merged
		float weight = 0.f;
	};

	struct Box {
		std::vector<std::uint32_t> items;
		float error = 0.f; /// Weighted squared distance of the items to their mean
		int axis = 0; /// Axis with the largest spread
	};

	inline std::uint32_t pack(const glm::rgba& c) {
		return std::uint32_t(c.r) | std::uint32_t(c.g) << 8 | std::uint32_t(c.b) << 16 | std::uint32_t(c.a) << 24;
	}

	inline glm::rgba unpack(std::uint32_t key) {
		return glm::rgba(key & 0xFF, (key >> 8) & 0xFF, (key >> 16) & 0xFF, (key >> 24) & 0xFF);
	}

	const std::array<float, 256>& srgbToLinear() {
		static const std::array<float, 256> table = [] {
			std::array<float, 256> out{};
			for (int i = 0; i < 256; ++i) {
				const float c = i / 255.f;
				out[i] = c <= 0.04045f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
			}
			return out;
		}();
		return table;
	}

	/// sRGB to OKLab, see https://bottosson.github.io/posts/oklab/
	glm::vec4 toPosition(const glm::vec4& rgba) {
		const auto& linear = srgbToLinear();
		const auto lin = [&linear](float v) { return linear[std::clamp(int(std::lround(v)), 0, 255)]; };
		const float r = lin(rgba.r), g = lin(rgba.g), b = lin(rgba.b);

		const float l = std::cbrt(0.4122214708f * r + 0.5363325363f * g + 0.0514459929f * b);
		const float m = std::cbrt(0.2119034982f * r + 0.6806995451f * g + 0.1073969566f * b);
		const float s = std::cbrt(0.0883024619f * r + 0.2817188376f * g + 0.6299787005f * b);

		return glm::vec4(
			0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s,
			1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s,
			0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s,
			rgba.a / 255.f);
	}

	inline float distance2(const glm::vec4& a, const glm::vec4& b) {
		const glm::vec4 d = a - b;
		return glm::dot(d, d);
	}

	void measure(Box& box, const std::vector<ColorPoint>& points) {
		float weight = 0.f;
		glm::vec4 mean(0.f);
		for (auto i : box.items) {
			weight += points[i].weight;
			mean += points[i].position * points[i].weight;
		}
		mean /= std::max(weight, std::numeric_limits<float>::min());

		glm::vec4 variance(0.f);
		for (auto i : box.items) {
			const glm::vec4 d = points[i].position - mean;
			variance += d * d * points[i].weight;
		}
		box.error = variance.x + variance.y + variance.z + variance.w;
		box.axis = 0;
		for (int a = 1; a < 4; ++a)
			if (variance[a] > variance[box.axis]) box.axis = a;
	}

	/// Median cut, the box with the largest error is split at the weighted median of its widest axis
	std::vector<Box> medianCut(const std::vector<ColorPoint>& points, std::size_t boxCount) {
		std::vector<Box> boxes(1);
		boxes[0].items.resize(points.size());
		for (std::uint32_t i = 0; i < points.size(); ++i) boxes[0].items[i] = i;
		measure(boxes[0], points);

		while (boxes.size() < boxCount) {
			auto splitIt = boxes.end();
			for (auto it = boxes.begin(); it != boxes.end(); ++it) {
				if (it->items.size() > 1 && (splitIt == boxes.end() || it->error > splitIt->error)) splitIt = it;
			}
			if (splitIt == boxes.end() || splitIt->error <= 0.f) break;

			Box& box = *splitIt;
			const int axis = box.axis;
			std::ranges::sort(box.items, {}, [&points, axis](std::uint32_t i) { return points[i].position[axis]; });

			float total = 0.f;
			for (auto i : box.items) total += points[i].weight;
			float sum = 0.f;
			std::size_t split = 1;
			for (; split < box.items.size() - 1; ++split) {
				sum += points[box.items[split - 1]].weight;
				if (sum >= total * 0.5f) break;
			}

			Box upper;
			upper.items.assign(box.items.begin() + split, box.items.end());
			box.items.resize(split);
			measure(box, points);
			measure(upper, points);
			boxes.push_back(std::move(upper));
		}
		return boxes;
	}

	std::size_t nearest(const glm::vec4& position, const std::vector<glm::vec4>& centroids) {
		std::size_t best = 0;
		float bestDistance = std::numeric_limits<float>::max();
		for (std::size_t c = 0; c < centroids.size(); ++c) {
			const float d = distance2(position, centroids[c]);
			if (d < bestDistance) {
				bestDistance = d;
				best = c;
			}
		}
		return best;
	}

	/// The distinct colors as points, merged into 5 bit buckets when there are too many
	std::vector<ColorPoint> createPoints(const std::vector<std::pair<std::uint32_t, std::uint32_t>>& colors) {
		std::vector<ColorPoint> points;
		if (colors.size() <= g_maxClusterPoints) {
			points.reserve(colors.size());
			for (const auto& [key, count] : colors) {
				const glm::vec4 color(unpack(key));
				points.push_back({ toPosition(color), color, std::sqrt(float(count)) });
			}
			return points;
		}

		struct Bucket { glm::dvec4 sum{ 0.0 }; std::uint64_t count = 0; };
		std::unordered_map<std::uint32_t, Bucket> buckets;
		for (const auto& [key, count] : colors) {
			const glm::rgba c = unpack(key);
			const std::uint32_t bucketKey = (c.r >> 3) | (c.g >> 3) << 5 | (c.b >> 3) << 10 | (c.a >> 4) << 15;
			Bucket& bucket = buckets[bucketKey];
			bucket.sum += glm::dvec4(c) * double(count);
			bucket.count += count;
		}
		points.reserve(buckets.size());
		for (const auto& [bucketKey, bucket] : buckets) {
			const glm::vec4 color(bucket.sum / double(bucket.count));
			points.push_back({ toPosition(color), color, std::sqrt(float(bucket.count)) });
		}
		return points;
	}

	/// Weighted mean colors of the clusters, refined by k-means
	std::vector<glm::rgba> createPalette(const std::vector<ColorPoint>& points, std::size_t colorCount) {
		std::vector<glm::vec4> centroids;
		for (const Box& box : medianCut(points, colorCount)) {
			glm::vec4 sum(0.f);
			float weight = 0.f;
			for (auto i : box.items) {
				sum += points[i].position * points[i].weight;
				weight += points[i].weight;
			}
			centroids.push_back(sum / std::max(weight, std::numeric_limits<float>::min()));
		}

		std::vector<std::size_t> assignment(points.size(), std::numeric_limits<std::size_t>::max());
		std::vector<glm::vec4> colorSums(centroids.size());
		std::vector<float> weights(centroids.size());
		for (int pass = 0; pass < g_kMeansPasses; ++pass) {
			bool changed = false;
			std::vector<glm::vec4> positionSums(centroids.size(), glm::vec4(0.f));
			std::ranges::fill(colorSums, glm::vec4(0.f));
			std::ranges::fill(weights, 0.f);

			for (std::size_t i = 0; i < points.size(); ++i) {
				const std::size_t c = nearest(points[i].position, centroids);
				changed |= assignment[i] != c;
				assignment[i] = c;
				positionSums[c] += points[i].position * points[i].weight;
				colorSums[c] += points[i].color * points[i].weight;
				weights[c] += points[i].weight;
			}
			for (std::size_t c = 0; c < centroids.size(); ++c) {
				if (weights[c] > 0.f) centroids[c] = positionSums[c] / weights[c];
			}
			if (!changed) break;
		}

		std::vector<glm::rgba> palette;
		palette.reserve(centroids.size());
		for (std::size_t c = 0; c < centroids.size(); ++c) {
			if (weights[c] <= 0.f) continue; //a cluster that lost all its colors
			palette.push_back(glm::rgba(glm::clamp(glm::round(colorSums[c] / weights[c]), glm::vec4(0.f), glm::vec4(255.f))));
		}
		return palette;
	}
}

PG_DisaColorReducer::Result PG_DisaColorReducer::reduce(const PG_RGBAImage& image, std::size_t maxColors) {
	maxColors = std::clamp<std::size_t>(maxColors, 2, 256);
	Result result;

	//every invisible pixel is the same color
	const auto keyOf = [](const glm::rgba& c) { return c.a == 0 ? 0u : pack(c); };

	std::unordered_map<std::uint32_t, std::uint32_t> histogram;
	for (const glm::rgba& c : image) ++histogram[keyOf(c)];

	const bool hasTransparent = histogram.contains(0u);
	std::vector<std::pair<std::uint32_t, std::uint32_t>> colors;
	colors.reserve(histogram.size());
	for (const auto& entry : histogram) {
		if (entry.first != 0u) colors.push_back(entry);
	}
	std::ranges::sort(colors, std::ranges::greater{}, &std::pair<std::uint32_t, std::uint32_t>::second);

	if (hasTransparent) result.palette.push_back(glm::rgba(0));
	std::unordered_map<std::uint32_t, unsigned char> indexOf;
	if (hasTransparent) indexOf[0u] = 0;

	if (colors.size() + result.palette.size() <= maxColors) {
		//fits as it is, the most used colors come first
		for (const auto& [key, count] : colors) {
			indexOf[key] = (unsigned char)result.palette.size();
			result.palette.push_back(unpack(key));
		}
	}
	else {
		const std::vector<glm::rgba> palette = createPalette(createPoints(colors), maxColors - result.palette.size());
		const std::size_t offset = result.palette.size();
		result.palette.insert(result.palette.end(), palette.begin(), palette.end());

		std::vector<glm::vec4> positions;
		positions.reserve(palette.size());
		for (const glm::rgba& c : palette) positions.push_back(toPosition(glm::vec4(c)));
		for (const auto& [key, count] : colors) {
			indexOf[key] = (unsigned char)(offset + nearest(toPosition(glm::vec4(unpack(key))), positions));
		}
	}

	result.indices.reserve(std::size_t(image.getWidth()) * image.getHeight());
	for (const glm::rgba& c : image) result.indices.push_back(indexOf[keyOf(c)]);
	return result;
}
