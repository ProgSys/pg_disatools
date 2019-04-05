#ifndef INCLUDE_PG_UTIL_PG_IMAGE_UTIL_H_
#define INCLUDE_PG_UTIL_PG_IMAGE_UTIL_H_

#include <Util/PG_VectorUtil.h>
namespace PG {
namespace UTIL {



template<typename T>
struct histogramEntry{
	unsigned int count = 0;
	unsigned int index = 0;
	T color;

	histogramEntry(){};
	histogramEntry(const T& v): color(v){};

	histogramEntry<T>& operator++(){
		count++;
		return this;
	}

	histogramEntry<T>& operator--(){
		if(count > 0)
			count--;
		return this;
	}
};


template<typename T>
void reduceColors(Image<T>& imageIn, unsigned int targetColors){
	IDuImage idImage(imageIn.getWidth(), imageIn.getHeight());
	std::vector<histogramEntry<T>>  hist;
	//build histogram
	auto it = idImage.begin();
	for(const T& color: imageIn){
		bool found = false;
		for(histogramEntry<T>& histcolor: hist){
			if(histcolor.color == color){
				histcolor.index = std::distance(hist.begin(), histcolor);
				(*it) = histcolor.index;
				histcolor.count++;
				found = true;
				break;
			}
		}
		if(!found){
			(*it) = hist.size();
			hist.push_back(histogramEntry<T>(color));
		}
		it++;
	}

	if(hist.size() <= targetColors) return;

}

} /* namespace UTIL */
} /* namespace PG */

#endif /* INCLUDE_PG_UTIL_PG_IMAGE_H_ */
