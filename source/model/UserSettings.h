/*
 * UserSettings.h
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#ifndef SOURCE_MODEL_USERSETTINGS_H_
#define SOURCE_MODEL_USERSETTINGS_H_

#include <stdint.h>
#include <stdbool.h>
#include "g_structs.h"


#define FRAM_SETTINGS_ADDRESS        0x0000
#define FRAM_SETTINGS_VERSION        0x0001

#if defined(__cplusplus)
extern "C" {
#endif

sUserParameters *user_settings_get(void);

#if defined(__cplusplus)
}
#endif


#if defined(__cplusplus)

class UserSettings {
public:
	UserSettings();

	bool isConfigValid(void);
	void resetConfig(void);
	bool writeConfig(void);

	void checkConfigVersion(void);

private:
	bool m_is_init = false;
	sUserParameters &m_params;
};

#endif // defined C++

#endif /* SOURCE_MODEL_USERSETTINGS_H_ */
