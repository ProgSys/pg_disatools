/*
 * Functions to uncompress S3 compression.
 *
 *  Copyright (C) 2016  ProgSys
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include <pg/files/PG_S3Compression.h>

#include <sstream>
#include <algorithm>
#include <type_traits>


#include <pg/util/PG_Array.h>
#include <pg/util/PG_Base.h>

#include <pg/util/PG_VectorUtil.h>

namespace PG {
namespace FILE {

using namespace UTIL;

inline int getColor(unsigned char a, unsigned char b, rgba& outRGBA){
	outRGBA.r = (b >> 3) & 0x1F;
	outRGBA.g = ((b << 3) & 0x38) + ((a >> 5) & 0x07);
	outRGBA.b = (a) & 0x1F;
	scaleRGB565(outRGBA);
	return (b << 8) + a;
}

void DXT1block::uncompress(std::vector<PG::UTIL::rgba>& outRGBAData) const{
	Array<rgba,4> c; // the 4 interpolated colors
	const int c0Val = getColor(color[0], color[1], c[0]);
	const int c1Val = getColor(color[2], color[3], c[1]);

	if(c0Val > c1Val){
		c[2] = interpolate(c[0],c[1], 1.0/3.0 );
		c[3] = interpolate(c[0],c[1], 2.0/3.0 );
	}else{
		c[2] = interpolate(c[0],c[1], 0.5 );
		c[3].a = 0;
	}

	//read lookup table and insert RGBA
	outRGBAData.resize(16); // block size 4x4
	for(unsigned int i = 4; i < 8; ++i){
		const unsigned int pos = (i-4)*4;
		outRGBAData[pos] = c[(color[i]) & 0x03];
		outRGBAData[pos+1] = c[(color[i]>>2) & 0x03];
		outRGBAData[pos+2] = c[(color[i]>>4) & 0x03];
		outRGBAData[pos+3] = c[(color[i]>>6) & 0x03];
	}


}

void DXT5block::uncompress(std::vector<PG::UTIL::rgba>& outRGBAData) const{
	Array<rgba,4> c; // the 4 interpolated colors
	const int c0Val = getColor(color[0], color[1], c[0]);
	const int c1Val = getColor(color[2], color[3], c[1]);

	c[2] = interpolate(c[0],c[1], 1.0/3.0 );
	c[3] = interpolate(c[0],c[1], 2.0/3.0 );

	//read lookup table and insert RGBA
	outRGBAData.resize(16); // block size 4x4
	for(unsigned int i = 4; i < 8; ++i){
		const unsigned int pos = (i-4)*4;
		outRGBAData[pos] = c[(color[i]) & 0x03];
		outRGBAData[pos+1] = c[(color[i]>>2) & 0x03];
		outRGBAData[pos+2] = c[(color[i]>>4) & 0x03];
		outRGBAData[pos+3] = c[(color[i]>>6) & 0x03];
	}


	// the 8 interpolated alpha values
	PG::UTIL::Array<unsigned char,8> a;
	a[0] = alpha[0];//( (alpha[0] >> 4) & 0x0F )*(255/15.0);
	a[1] = alpha[1];//(alpha[0] & 0x0F)*(255/15.0);

	if(a[0] > a[1] ){
		a[2] =  interpolate(a[0], a[1], 1.0/7.0);
		a[3] =  interpolate(a[0], a[1], 2.0/7.0);
		a[4] =  interpolate(a[0], a[1], 3.0/7.0);
		a[5] =  interpolate(a[0], a[1], 4.0/7.0);
		a[6] =  interpolate(a[0], a[1], 5.0/7.0);
		a[7] =  interpolate(a[0], a[1], 6.0/7.0);
	}else{
		a[2] =  interpolate(a[0], a[1], 1.0/5.0);
		a[3] =  interpolate(a[0], a[1], 2.0/5.0);
		a[4] =  interpolate(a[0], a[1], 3.0/5.0);
		a[5] =  interpolate(a[0], a[1], 4.0/5.0);
		a[6] = 0;
		a[7] = 255;
	}

	/*
	for(unsigned char c: a){
		std::cout << (int)c <<" ";
	}
	std::cout << std::endl;
	*/

	//read alpha lookup table and set A
	unsigned int start_bit = 16; //(offset to the start of lookup)
	//TODO: find something easyer?
	for(unsigned int i = 0; i < 16; ++i){
		const unsigned int t1 = start_bit/8;
		const unsigned int end_bit = start_bit+2;
		const unsigned int t2 = (end_bit)/8;
		const unsigned int start_bit_rel = start_bit - int(start_bit/8.0)*8;

		if(t1 == t2){ //is the value between two chars?
			const unsigned int u = (alpha[t1]>>start_bit_rel) & 0x07;
			outRGBAData[i].a = a[u];
		}else{
			if(start_bit_rel == 6){
				const unsigned int u = (alpha[t1]>>6) | ( (alpha[t2]<<2) & 0x01);
				outRGBAData[i].a = a[u];
			}else{
				const unsigned int u = (alpha[t1]>>7) | (alpha[t2]<<1 & 0x06);
				outRGBAData[i].a = a[u];
			}

		}
		start_bit+=3;
	}

}

} /* namespace FILE */
} /* namespace PG */
