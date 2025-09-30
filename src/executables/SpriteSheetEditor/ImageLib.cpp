/*
 * ImageLib.cpp
 *
 *  Created on: 17.12.2017
 *      Author: ProgSys
 */

#include <ImageLib.h>
#include <QList>
#include <QMap>
#include <QDebug>
#include <cassert>
#include <cmath>
#include <unordered_set>
#include <glm/glm/glm.hpp>

using namespace std;

static inline double euclideanDis2(QRgb A, QRgb B) {
	return pow(qRed(B) - qRed(A), 2) + pow(qGreen(B) - qGreen(A), 2) + pow(qBlue(B) - qBlue(A), 2) + pow(qAlpha(B) - qAlpha(A), 2);
}

static inline double euclideanDis(QRgb A, QRgb B){
	return sqrt(euclideanDis2(A, B));
}

bool operator<( QColor const& c1,  QColor const & c2){
	if(c1.red() < c2.red()) return true;
	if(c1.red() == c2.red()){
		if(c1.green() < c2.green()) return true;
		if(c1.green() == c2.green()){
			if(c1.blue() < c2.blue()) return true;
			if(c1.blue() == c2.blue())
				if(c1.alpha() < c2.alpha()) return true;
		}
	}
	return false;
}

namespace ImageLib {

struct HistogrammRow {
	QRgb color;
	mutable std::size_t count = 0;

	HistogrammRow() = default;
	HistogrammRow(QRgb&& colorIn) : color(std::move(colorIn)) {};
	HistogrammRow(const QRgb& colorIn) : color(colorIn) {};

	bool operator==(const HistogrammRow& other) const {
		return color == other.color;
	}

	size_t operator()(const HistogrammRow& v) const {
		return qHash(v.color);
	}
};

struct Histogramm{
	std::unordered_set<HistogrammRow, HistogrammRow> histo;

	Histogramm(const QImage& image) { fill(image);};

	void create(const QImage& image){
		histo.clear();
		fill(image);
	}

	void fill(const QImage& image){
		for(int y = 0; y < image.height(); ++y){
			for(int x = 0; x < image.width(); ++x){
				(*this)[image.pixel(x,y)] += 1;
			}
		}
	}

	std::size_t& operator[](const QRgb& color){
		auto findIt = histo.find(color);
		return histo.insert(color).first->count;
	}
	std::size_t operator[](const QRgb& color) const{
		auto findIt = histo.find(color);
		return findIt == histo.end()? 0: findIt->count;
	}
	std::size_t& operator[](const QColor& color){
		return histo.insert(color.rgba()).first->count;
	}
	unsigned int operator[](const QColor& color) const {
		auto findIt = histo.find(color.rgba());
		return findIt == histo.end() ? 0 : findIt->count;
	}

	void mergeFirstWithClosest(std::unordered_map<QRgb, QRgb>& replaceMap);
	void mergeClusters(std::unordered_map<QRgb, QRgb>& replaceMap);
	void mapToClosestColor(QMap<QRgb, int>& mapping, const QVector<QColor>& targetColors) const;

	int size() const { return histo.size(); }

};

void Histogramm::mergeFirstWithClosest(std::unordered_map<QRgb, QRgb>& replaceMap){
	if(size() <= 1) return;
	//find color that is used the least
	auto minIt = std::min_element(histo.begin(), histo.end(), [](const HistogrammRow& a, const HistogrammRow& b) ->bool {
		return a.count < b.count;
		});
	HistogrammRow toMerge = *minIt;
	histo.erase(minIt);

	//find closest color
	auto mergeWithIt = std::min_element(histo.begin(), histo.end(), [&toMerge](const HistogrammRow& a, const HistogrammRow& b) ->bool{
		auto compDistance = [&toMerge](const HistogrammRow& v) {
			if (qAlpha(v.color) < 10 && qAlpha(toMerge.color) < 10) return 0.0;
			return euclideanDis2(v.color, toMerge.color);//CIEDE2000::CIEDE2000(LAB(firstRow.color), LAB(row.color));
		};
		const double distanceA = compDistance(a);
		const double distanceB = compDistance(b);
		return (distanceA == distanceB && a.count > b.count) || distanceA < distanceB;
	});
	mergeWithIt->count += toMerge.count;

	for (auto& [key, color] : replaceMap) {
		if (color == toMerge.color) color = mergeWithIt->color;
	}
	replaceMap[toMerge.color] = mergeWithIt->color;
}

void Histogramm::mergeClusters(std::unordered_map<QRgb, QRgb>& replaceMap) {
	struct ColorIndex {
		ColorIndex() = default;
		ColorIndex(ColorIndex&&) = default;
		ColorIndex(const ColorIndex&) = default;

		ColorIndex(const QRgb c) {
			constexpr static int chunkSize = 3;
			if (qAlpha(c) == 0) {
				index = 0;
				return;
			}
			index = 1 + (qRed(c) / chunkSize) + qGreen(c) * 255 + (qBlue(c) / chunkSize) * (255 * 255) + (qAlpha(c) / chunkSize) * (255 * 255 * 255);
		}

		glm::ivec4 getChunkCord() const {
			glm::ivec4 out(-1);
			if (index == 0) return out;
			auto i = index - 1;
			out.r = i % 255;
			i /= 255;
			out.g = i % 255;
			i /= 255;
			out.b = i % 255;
			i /= 255;
			out.a = i % 255;
			return out;
		}

		void setChunkCord(glm::ivec4 v) {
			if (v.r < 0 || v.g < 0 || v.b < 0 || v.a < 0) {
				index = 0;
				return;
			}
			index = 1 + v.r + v.g * 255 + v.b * (255 * 255) + v.a * (255 * 255 * 255);
		}

		ColorIndex getNeighbor(glm::ivec4 offset) const {
			constexpr static int chunkSize = 3;
			if (index == 0) return *this;
			const auto cord = getChunkCord() + offset;
			//is out of bounds?
			if (cord.r < 0 || cord.g < 0 || cord.b < 0 || cord.a < 0 ||
				cord.r >= chunkSize || cord.g >= chunkSize || cord.b >= chunkSize || cord.a >= chunkSize)
				return *this;
			ColorIndex out;
			out.setChunkCord(cord);
			return out;
		}

		bool operator==(ColorIndex other) const { return index == other.index; }
		bool operator!=(ColorIndex other) const { return index != other.index; }
		bool operator<(ColorIndex other) const { return index < other.index; }
		bool operator<=(ColorIndex other) const { return index <= other.index; }
		bool operator>(ColorIndex other) const { return index > other.index; }
		bool operator>=(ColorIndex other) const { return index >= other.index; }

		std::size_t index;
	};

	std::map<ColorIndex, std::vector<QRgb>> clusters;
	for (const auto& it : histo) {
		clusters[it.color].push_back(it.color);
	}

	for (auto& [index, colors] : clusters) {
		double avgR = 0, avgG = 0, avgB = 0, avgA = 0;
		size_t n = 0;
		int sum = 0;

		for (QRgb color : colors) {
			++n;
			avgR += (qRed(color) - avgR) / n ;
			avgG += (qGreen(color) - avgG) / n;
			avgB += (qBlue(color) - avgB) / n;
			avgA += (qAlpha(color) - avgA) / n;
			auto findIt = histo.find(color);
			sum += findIt->count;
			histo.erase(findIt);
		}

		QRgb average = qRgba(static_cast<int>(avgR + 0.5), static_cast<int>(avgG + 0.5), static_cast<int>(avgB + 0.5), static_cast<int>(avgA + 0.5));
		auto it = histo.insert(average);
		it.first->count += sum;

		for (QRgb color : colors) {
			replaceMap[color] = average;
		}
		replaceMap.erase(average);
	}

}

void Histogramm::mapToClosestColor(QMap<QRgb, int>& mapping, const QVector<QColor>& targetColors) const{
	for(const HistogrammRow& row: histo){
		int findIndex = -1;
		double findDisstance = std::numeric_limits<double>::max();
		//const LAB histoRowTarget(row.color);
		//qDebug()<<__LINE__<<"=== Find similar to: ("<<qRed(row.color.rgba())<<", "<<qGreen(row.color.rgba())<<", "<<qBlue(row.color.rgba())<<", "<<qAlpha(row.color.rgba())<<") at "<<i <<" ===" ;
		for(int j = 0; j < targetColors.size(); j++){
			const QColor& targetColor = targetColors[j];
			if((qAlpha(row.color) == 0 && qAlpha(targetColor.rgba()) == 0) ||
					(row.color == targetColor.rgba())){
				findIndex = j;
				//qDebug()<<__LINE__<<"FOUND to: ("<<qRed(targetColor.rgba())<<", "<<qGreen(targetColor.rgba())<<", "<<qBlue(targetColor.rgba())<<", "<<qAlpha(targetColor.rgba())<<") at "<<j<<" from "<<targetColors.size()  ;
				break;
			}
			const double tempDistance = euclideanDis( targetColor.rgba(),row.color);//CIEDE2000::CIEDE2000(LAB(targetColor), histoRowTarget);
			//qDebug()<<__LINE__<<"Campare to: ("<<qRed(targetColor.rgba())<<", "<<qGreen(targetColor.rgba())<<", "<<qBlue(targetColor.rgba())<<", "<<qAlpha(targetColor.rgba())<<") at "<<j<<" from "<<targetColors.size()<<" val: "<<tempDistance  ;
			if(tempDistance < findDisstance){
				findDisstance = tempDistance;
				findIndex = j;
			}
		}
		//qDebug()<<__LINE__<<"Map ("<<qRed(row.color.rgba())<<", "<<qGreen(row.color.rgba())<<", "<<qBlue(row.color.rgba())<<", "<<qAlpha(row.color.rgba())<<") to "<<findIndex ;
		mapping[row.color] = findIndex;
	}
}

static inline void replaceColors(QImage& image, std::unordered_map<QRgb, QRgb>& replaceMap) {
	//replace in image
	int breakCount = 0;
	for (int y = 0; y < image.height(); ++y) {
		for (int x = 0; x < image.width(); ++x) {
			const QRgb color = image.pixel(x, y);
			auto findIt = replaceMap.find(color);
			if (findIt == replaceMap.end()) continue;
			breakCount = image.height() * image.width();
			while (--breakCount > 0) {
				auto findIt2 = replaceMap.find(findIt->second);
				if (findIt2 == replaceMap.end()) break;
				findIt = findIt2;
			}
			image.setPixel(x, y, findIt->second);
		}
	}
}

static inline void replaceColorsSimple(QImage& image, const std::unordered_map<QRgb, QRgb>& replaceMap) {
	//replace in image
	int breakCount = 0;
	for (int y = 0; y < image.height(); ++y) {
		for (int x = 0; x < image.width(); ++x) {
			const QRgb color = image.pixel(x, y);
			auto findIt = replaceMap.find(color);
			if (findIt == replaceMap.end()) continue;
			image.setPixel(x, y, findIt->second);
		}
	}
}

void reduceColors(QImage& image, unsigned int targetColors){
	if(targetColors == 0) return;

	//create Histogramm
	Histogramm histogramm(image);
	if(histogramm.size() <= targetColors) return;

	//map to replace color in key with given value
	std::unordered_map<QRgb, QRgb> replaceMap;

	if (histogramm.size() > 2048 && targetColors < 2048) {
		histogramm.mergeClusters(replaceMap);
		replaceColors(image, replaceMap);
		replaceMap.clear();
	}

	if(histogramm.size() <= targetColors) return;

	while(histogramm.size() > targetColors){
		if (histogramm.size() % 10 == 0) qDebug()<<__LINE__<<": Image merge"<<histogramm.size()<<" to "<<targetColors;
		histogramm.mergeFirstWithClosest(replaceMap);
	}

	replaceColorsSimple(image, replaceMap);
}

QString toString(const QColor& color){
	const auto rgba = color.rgba();
	return "("+QString::number( qRed(rgba) )+", "+ QString::number( qGreen(rgba) )+", "+ QString::number( qBlue(rgba) )+", "+ QString::number( qAlpha(rgba) ) +")";
}

void reassignColors(QImage& image, const QVector<QColor>& targetColors){
	if(targetColors.size() == 0) return;
	Histogramm histogramm(image);
	QMap<QRgb, int> mapping;
	histogramm.mapToClosestColor(mapping, targetColors);

	//replace in image
	for(int y = 0; y < image.height(); y++){
		for(int x = 0; x < image.width(); x++){
			const QRgb color = image.pixel(x,y);
			image.setPixel(x,y, targetColors[mapping[color]].rgba());
		}
	}
}

} /* namespace ImageLib */
