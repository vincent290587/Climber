/*
 * MovingAVerage.h
 *
 *  Created on: 25 janv. 2020
 *      Author: Vincent
 */

#ifndef LIBRARIES_UTILS_SMA_H_
#define LIBRARIES_UTILS_SMA_H_

#include <list>
#include <stdint.h>

class SMA {
public:
	SMA(uint16_t nb_samples, uint16_t min_samples) : m_nb_samples(nb_samples), m_min_samples(min_samples) {};

	void clear(void) {
		m_val.clear();
	}

	void addSample(float sam) {
		m_val.push_back(sam);
		if (m_val.size() > m_nb_samples) {
			m_val.pop_front();
		}
	}

	uint16_t getValue(float &ret_val) {

		if (m_val.size() >= m_min_samples) {
			float res = 0.f;

			for (auto& val : m_val) {
				res += val;
			}

			res /= m_val.size();
			ret_val = res;

			return m_val.size();
		}

		return 0;
	}

private:
	const uint16_t m_nb_samples;
	const uint16_t m_min_samples;

	std::list<float> m_val;
};


#endif /* LIBRARIES_UTILS_SMA_H_ */
