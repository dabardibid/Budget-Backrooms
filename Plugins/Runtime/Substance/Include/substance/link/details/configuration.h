/** @file configuration.h
    @brief Contains the functions for reading a configuration and the
           structures to store them.
    @author Galen Helfter - Allegorithmic
    @date 20181019
    @copyright Allegorithmic. All rights reserved.
*/

#ifndef _SUBSTANCE_LINK_DETAILS_CONFIGURATION_H
#define _SUBSTANCE_LINK_DETAILS_CONFIGURATION_H

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

unsigned int link_parse_configuration_file(const char *filepath);
unsigned int link_parse_configuration_buffer(const char *buffer);

/* Set the application name. This should only be called prior to
 * or at initialization, as it should be immutable until shutdown. */
unsigned int link_set_application_name(const char *name);

/* Acquire the application name for internal use */
const char* link_get_application_name(void);

void link_clear_application_name(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _SUBSTANCE_LINK_DETAILS_CONFIGURATION_H */
