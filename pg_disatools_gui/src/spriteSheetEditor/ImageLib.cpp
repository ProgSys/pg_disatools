/*
 * ImageLib.cpp
 *
 *  Created on: 17.12.2017
 *      Author: ProgSys
 */

#include <spriteSheetEditor/ImageLib.h>
#include <QList>
#include <QMap>
#include <QDebug>
#include <cassert>

double euclideanDis(QRgb A, QRgb B){
	return sqrt( pow( qRed(B)-qRed(A),2) + pow( qGreen(B)-qGreen(A),2)+ pow( qBlue(B)-qBlue(A),2) +pow( qAlpha(B)-qAlpha(A),2));
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

struct HistogrammRow{
	QColor color;
	unsigned int count;
};

bool histogrammRowLessThan(const HistogrammRow &v1, const HistogrammRow &v2){
    return v1.count < v2.count;
}

struct Histogramm{

	QList<HistogrammRow> histo;

	Histogramm(const QImage& image) { fill(image);};

	void create(const QImage& image){
		histo.clear();
		fill(image);
	}

	void fill(const QImage& image){
		for(int y = 0; y < image.height(); y++){
			for(int x = 0; x < image.width(); x++){
				(*this)[image.pixel(x,y)] += 1;
			}
		}
	}

	void sort(){qSort(histo.begin(), histo.end(), histogrammRowLessThan);}

	unsigned int& operator[](const QRgb& color){
		for(HistogrammRow& row: histo)
			if(row.color.rgba() == color) return row.count;
		histo.push_back({QColor(qRed(color), qGreen(color), qBlue(color), qAlpha(color)), 0});
		return histo.back().count;
	}
	unsigned int operator[](const QRgb& color) const{
		for(const HistogrammRow& row: histo)
			if(row.color.rgba() == color) return row.count;
		return 0;
	}
	unsigned int& operator[](const QColor& color){
		for(HistogrammRow& row: histo)
			if(row.color.rgba() == color.rgba()) return row.count;
		histo.push_back({color, 0});
		return histo.back().count;
	}
	unsigned int operator[](const QColor& color) const {
		for(const auto& row: histo)
			if(row.color.rgba() == color.rgba()) return row.count;
		return 0;
	}

	void mergeFirstWithClosest(QImage& targetImage);
	void mapToClosestColor(QMap<QRgb, int>& mapping, const QVector<QColor>& targetColors) const;

	int size() const { return histo.size(); }

	HistogrammRow& first()  { return histo.first(); }
	const HistogrammRow& first() const { return histo.first(); }
	HistogrammRow& last()  { return histo.last(); }
	const HistogrammRow& last() const { return histo.last(); }

	//last
	QList<HistogrammRow>::iterator begin()  { return histo.begin(); }
	QList<HistogrammRow>::iterator end()  { return histo.end(); }
	QList<HistogrammRow>::const_iterator begin() const { return histo.begin(); }
	QList<HistogrammRow>::const_iterator end() const { return histo.end(); }
};

void Histogramm::mergeFirstWithClosest(QImage& targetImage){
	if(size() <= 1) return;
	//find closest color
	int findIndex = -1;
	double findDisstance = std::numeric_limits<double>::max();
	HistogrammRow findRow;
	const HistogrammRow firstRow = histo.first();
	//qDebug()<<__LINE__<<"Find similar to: ("<<qRed(firstRow.color.rgba())<<", "<<qGreen(firstRow.color.rgba())<<", "<<qBlue(firstRow.color.rgba())<<", "<<qAlpha(firstRow.color.rgba())<<") from "<<size()  ;
	for(int i = 1; i < size(); i++){
		HistogrammRow& row = histo[i];
		//special case for alpha values
		if(firstRow.color.alpha() < 10 && row.color.alpha() == 0){
			findIndex = i;
			findRow = row;
			break;
		}
		const double tempDistance = euclideanDis(firstRow.color.rgba(), row.color.rgba());//CIEDE2000::CIEDE2000(LAB(firstRow.color), LAB(row.color));
		if(tempDistance < findDisstance){
			findDisstance = tempDistance;
			findIndex = i;
			findRow = row;
		}
	}
	//qDebug()<<__LINE__<<"Replace at "<<findIndex<<" out of "<<histo.size() ;
	//merge historgramm colors
	const QRgb replaceColor = histo[findIndex].color.rgba();
	//qDebug()<<__LINE__<<"Replace: ("<<qRed(replaceColor)<<", "<<qGreen(replaceColor)<<", "<<qBlue(replaceColor)<<", "<<qAlpha(replaceColor)<<") at "<<findIndex  ;
	assert(replaceColor != firstRow.color.rgba());
	histo[findIndex].count += firstRow.count;
	histo.pop_front();

	//replace in image
	for(int y = 0; y < targetImage.height(); y++){
		for(int x = 0; x < targetImage.width(); x++){
			const QRgb color = targetImage.pixel(x,y);
			//qDebug()<<__LINE__<<"Try replace at: ("<<x<<", "<<y<<") color: ("<<qRed(color)<<", "<<qGreen(color)<<", "<<qBlue(color)<<", "<<qAlpha(color)<<")"  ;
			if( color == firstRow.color.rgba()){
				targetImage.setPixel(x,y, replaceColor);
				//qDebug()<<__LINE__<<"Replace at: ("<<x<<", "<<y<<") color: ("<<qRed(color)<<", "<<qGreen(color)<<", "<<qBlue(color)<<", "<<qAlpha(color)<<")"  ;
			}
		}
	}

	sort();
}



void Histogramm::mapToClosestColor(QMap<QRgb, int>& mapping, const QVector<QColor>& targetColors) const{
	for(int i = 0; i < size(); i++){
		const HistogrammRow& row = histo[i];
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
	histogramm.sort();

	while(histogramm.size() > targetColors){
		//qDebug()<<__LINE__<<": Image merge"<<histogramm.size()<<" to "<<targetColors;
		histogramm.mergeFirstWithClosest(image);
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
