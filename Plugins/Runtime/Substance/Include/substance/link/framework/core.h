//! @file core.h
//! @brief Defines core operations for Substance link
//! @author Galen Helfter - Allegorithmic
//! @date 20190411
//! @copyright Allegorithmic. All rights reserved.

#pragma once

namespace Alg
{
namespace Link
{
namespace Framework
{
class Application;

//! @brief Acquire the version of the Substance Link library
//! @return Pointer to the string representation of the version
const char* version();

//! @brief Register an application with the core system
//! @param application Pointer to a persistent application object
//! @return True on success, false on failure. Fails if the application
//          is already registered or if the system is already initialized.
bool registerApplication(Application* application);

//! @brief Initialize the core link system
//! @param applicationName String identifier for this application
//! @param module Pointer to the dynamically loaded library
//! @return True on success, false on failure
bool init(const std::string& applicationName, void* module = nullptr);

//! @brief Shut down the core link systems
//! @return True on success, false on failure
bool shutdown();

//! @brief Open a TCP connection at the given port
//! @param port Integer port to open the connection at
//! @param context Pointer to return the context through
//! @return True on success, false on failure
bool openTcpConnection(unsigned int port, unsigned int* context);

//! @brief Establish a TCP connection at the port
//! @param port Integer port to open the connection at
//! @param context Pointer to return the context through
//! @return True on success, false on failure
bool establishTcpConnection(unsigned int port, unsigned int* context);

//! @brief Open a default TCP connection
//! @param[out] context Pointer to return the context throughh
//! @return True on success, false on failure
bool openDefaultTcp(unsigned int* context);

//! @brief Perform a broadcast tcp connection, finding other Link instances
//! @return True on success, false on failure
bool broadcastTcp();

//! @brief Open a default unix connection
//! @param[out] context Pointer to return the context throughh
//! @return True on success, false on failure
bool openDefaultUnix(unsigned int* context);

//! @brief Perform a broadcast unix connection, finding other Link instances
//! @return True on success, false on failure
bool broadcastUnix();

//! @brief Open a default, system-specific connection
//! @param[out] context Pointer to return the context throughh
//! @return True on success, false on failure
bool openDefault(unsigned int* context);

//! @brief Perform a default broadcast connection, finding other Link instances
//! @return True on success, false on failure
bool broadcastDefault();
} // namespace Framework
} // namespace Link
} // namespace Alg
