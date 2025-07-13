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

using namespace std;

double euclideanDis(QRgb A, QRgb B){
	return sqrt(pow( qRed(B)-qRed(A),2) + pow( qGreen(B)-qGreen(A),2)+ pow( qBlue(B)-qBlue(A),2) +pow( qAlpha(B)-qAlpha(A),2));
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

struct QColorHash {
	static inline size_t operator()(const QColor& v) {
		return std::hash<unsigned int>{}(v.rgba());
	}
};

struct HistogrammRow {
	QColor color;
	mutable std::size_t count = 0;

	HistogrammRow() = default;
	HistogrammRow(QColor&& colorIn) : color(std::move(colorIn)) {};
	HistogrammRow(const QColor& colorIn) : color(colorIn) {};

	bool operator==(const HistogrammRow& other) const {
		return (color.alpha() == 0 && other.color.alpha() == 0) || color == other.color;
	}

	size_t operator()(const HistogrammRow& v) const {
		return QColorHash()(v.color);
	}
};


bool histogrammRowLessThan(const HistogrammRow& v1, const HistogrammRow& v2) {
	return v1.count < v2.count;
}

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
		auto findIt = histo.find(QColor(color));
		return histo.insert(QColor(color)).first->count;
	}
	std::size_t operator[](const QRgb& color) const{
		auto findIt = histo.find(QColor(color));
		return findIt == histo.end()? 0: findIt->count;
	}
	std::size_t& operator[](const QColor& color){
		return histo.insert(color).first->count;
	}
	unsigned int operator[](const QColor& color) const {
		auto findIt = histo.find(color);
		return findIt == histo.end() ? 0 : findIt->count;
	}

	void mergeFirstWithClosest(std::unordered_map<QColor, QColor, QColorHash>& replaceMap);
	void mapToClosestColor(QMap<QRgb, int>& mapping, const QVector<QColor>& targetColors) const;

	int size() const { return histo.size(); }

};

void Histogramm::mergeFirstWithClosest(std::unordered_map<QColor, QColor, QColorHash>& replaceMap){
	if(size() <= 1) return;
	//find color that is used the least
	auto minIt = std::min_element(histo.begin(), histo.end(), [](const HistogrammRow& a, const HistogrammRow& b) ->bool {
		return a.count < b.count;
		});
	HistogrammRow toMerge = std::move(*minIt);
	histo.erase(minIt);

	//find closest color
	auto mergeWithIt = std::min_element(histo.begin(), histo.end(), [&toMerge](const HistogrammRow& a, const HistogrammRow& b) ->bool{
		auto compDistance = [&toMerge](const HistogrammRow& v) {
			if (v.color.alpha() < 10 && toMerge.color.alpha() == 0) return 0.0;
			return euclideanDis(v.color.rgba(), toMerge.color.rgba());//CIEDE2000::CIEDE2000(LAB(firstRow.color), LAB(row.color));
		};
		const double distanceA = compDistance(a);
		const double distanceB = compDistance(b);
		return (distanceA == distanceB && a.count > b.count) || distanceA < distanceB;
	});
	mergeWithIt->count += toMerge.count;

	replaceMap[toMerge.color] = mergeWithIt->color;
}



void Histogramm::mapToClosestColor(QMap<QRgb, int>& mapping, const QVector<QColor>& targetColors) const{
	for(const HistogrammRow& row: histo){
		int findIndex = -1;
		double findDisstance = std::numeric_limits<double>::max();
		//const LAB histoRowTarget(row.color);
		//qDebug()<<__LINE__<<"=== Find similar to: ("<<qRed(row.color.rgba())<<", "<<qGreen(row.color.rgba())<<", "<<qBlue(row.color.rgba())<<", "<<qAlpha(row.color.rgba())<<") at "<<i <<" ===" ;
		for(int j = 0; j < targetColors.size(); j++){
			const QColor& targetColor = targetColors[j];
			if((row.color.alpha() == 0 && targetColor.alpha() == 0) ||
					(row.color.rgba() == targetColor.rgba())){
				findIndex = j;
				//qDebug()<<__LINE__<<"FOUND to: ("<<qRed(targetColor.rgba())<<", "<<qGreen(targetColor.rgba())<<", "<<qBlue(targetColor.rgba())<<", "<<qAlpha(targetColor.rgba())<<") at "<<j<<" from "<<targetColors.size()  ;
				break;
			}
			const double tempDistance = euclideanDis( targetColor.rgba(),row.color.rgba());//CIEDE2000::CIEDE2000(LAB(targetColor), histoRowTarget);
			//qDebug()<<__LINE__<<"Campare to: ("<<qRed(targetColor.rgba())<<", "<<qGreen(targetColor.rgba())<<", "<<qBlue(targetColor.rgba())<<", "<<qAlpha(targetColor.rgba())<<") at "<<j<<" from "<<targetColors.size()<<" val: "<<tempDistance  ;
			if(tempDistance < findDisstance){
				findDisstance = tempDistance;
				findIndex = j;
			}
		}
		//qDebug()<<__LINE__<<"Map ("<<qRed(row.color.rgba())<<", "<<qGreen(row.color.rgba())<<", "<<qBlue(row.color.rgba())<<", "<<qAlpha(row.color.rgba())<<") to "<<findIndex ;
		mapping[row.color.rgba()] = findIndex;
	}
}


void reduceColors(QImage& image, unsigned int targetColors){
	if(targetColors == 0) return;

	//create Histogramm
	Histogramm histogramm(image);
	if(histogramm.size() <= targetColors) return;

	std::unordered_map<QColor, QColor, QColorHash> replaceMap;
	while(histogramm.size() > targetColors){
		qDebug()<<__LINE__<<": Image merge"<<histogramm.size()<<" to "<<targetColors;
		histogramm.mergeFirstWithClosest(replaceMap);
	}

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
			image.setPixel(x, y, findIt->second.rgba());
		}
	}
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
