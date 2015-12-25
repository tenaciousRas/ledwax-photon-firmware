#ifndef LEDWAX_H
#define LEDWAX_H

class LEDWaxPhotonUtil {

public:

	LEDWaxPhotonUtil();
	~LEDWaxPhotonUtil();

	/**
	 * @return true if pre starts with str
	 */
	bool LEDWaxPhotonUtil::startsWith(const char *pre, const char *str) {
		size_t lenpre = strlen(pre), lenstr = strlen(str);
		return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
	}

	/**
	 * Create a 24 bit color value from R,G,B
	 */
	uint32_t LEDWaxPhotonUtil::rgbColor(uint8_t r, uint8_t g, uint8_t b) {
		uint32_t c;
		c = r;
		c <<= 8;
		c |= g;
		c <<= 8;
		c |= b;
		return c;
	}

	/**
	 * Input a value 0 to 255 to get a color value.
	 * The colours are a transition r - g -b - back to r
	 */
	uint32_t LEDWaxPhotonUtil::wheel(uint8_t wheelPos) {
		if (wheelPos < 85) {
			return rgbColor(wheelPos * 3, 255 - wheelPos * 3, 0);
		} else if (wheelPos < 170) {
			wheelPos -= 85;
			return rgbColor(255 - wheelPos * 3, 0, wheelPos * 3);
		} else {
			wheelPos -= 170;
			return rgbColor(0, wheelPos * 3, 255 - wheelPos * 3);
		}
	}

private:
}

