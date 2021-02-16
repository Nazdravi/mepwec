/*! \file playersvc.h **********************************************************
 *
 * \brief Zentrale Programmlogik - Strukturen und Funktionen
 *
 * \author		Arndt Kritzner, Logic Way GmbH
 * \version		30. Dezember 2020	Initialversion
 *
 *****************************************************************************/

#ifndef PLAYERCTL_H_
#define PLAYERCTL_H_

#include <stdbool.h>
#include <stdint.h>

#define CONSOLE_RUN_FILE "/var/run/playersvc.pts"
#define TERMINAL_GROUP "playerctl"
#define CONFIG_FILE "/etc/playersvc.conf"
#define CONFIG_PATHS_LENGTH 8192

#define CONSOLE_LF  "\r\n"
#define CONSOLE_LINE_BUF_LEN 1024

/*!@addtogroup util
*/
/*!@{*/

/*! \name Shell Commands
 */
//! @{
#define CMD_NONE		0x00
#define CMD_EXIT		0x01
#define CMD_ECHO		0xff
//! @}

/*! \name Special Char Values
 */
//! @{
#if !defined(CR)
#define CR                    '\r'
#endif
#if !defined(LF)
#define LF                    '\n'
#endif
#define CRLF                  "\r\n"
//! @}

/*! \name String Values for Commands
 */
//! @{
#define STR_EXIT		"exit|ex|x|quit|q"
#define STR_ECHO		"what?"
//! @}

/*! \name String Messages
 */
//! @{
#define MSG_PROMPT            "$>"
#define MSG_ER_CMD_NOT_FOUND  STR_ECHO"\r\n"
#define MSG_EXIT	" ... bye\r\n"
 
//! @}

#define SPECIFIC_COMMANDS_DEFINITIONS
// ====================================================================================================================
#include "specific-commands.h"
#undef SPECIFIC_COMMANDS_DEFINITIONS

#endif
