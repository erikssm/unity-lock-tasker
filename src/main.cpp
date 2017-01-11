#include <cstdio>
#include <dbus/dbus.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include "DBusError.hpp"

namespace
{

bool IsDesktopLocked()
{
    constexpr int dBusDefaultTimeout { -1 };
    constexpr char dbusTarget[] { "org.gnome.ScreenSaver" };
    constexpr char dbusPath[] { "/com/canonical/Unity/Session" };
    constexpr char dbusInterface[] { "com.canonical.Unity.Session" };
    constexpr char dbusMethod[] { "IsLocked" };

    DBusMessage* msg {};
    DBusMessageIter args;
    DBusConnection* conn {};
    ult::DBusError err;
    DBusPendingCall* pending {};
    int ret;
    bool result { false };

    dbus_error_init( err );

    conn = dbus_bus_get( DBUS_BUS_SESSION, err );
    if ( err.IsError() )
    {
        throw std::runtime_error { "dbus connection error: " + err.GetMessage() };
    }
    if ( NULL == conn )
    {
        throw std::runtime_error { "dbus connection error (NULL)" };
    }

    // request our name on the bus
    ret = dbus_bus_request_name( conn, "unitylock.tasker", DBUS_NAME_FLAG_REPLACE_EXISTING, err );
    if ( err.IsError() )
    {
        throw std::runtime_error { "dbus connection error: " + err.GetMessage() };
    }
    if ( DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret )
    {
        throw std::runtime_error { "dbus_bus_request_name failed" };
    }

    msg = dbus_message_new_method_call(
            dbusTarget,
            dbusPath,
            dbusInterface,
            dbusMethod );
    if ( NULL == msg )
    {
        throw std::runtime_error { "out of memory" };
    }

    if ( !dbus_connection_send_with_reply( conn, msg, &pending, dBusDefaultTimeout ) )
    {
        throw std::runtime_error { "out of memory" };
    }
    if ( NULL == pending )
    {
        throw std::runtime_error { "connection is disconnected or trying to send Unix file descriptors on a connection that does not support them" };
    }
    dbus_connection_flush( conn );

    dbus_message_unref( msg );

    dbus_pending_call_block( pending );

    msg = dbus_pending_call_steal_reply( pending );
    if ( NULL == msg )
    {
        throw std::runtime_error { "failed to retrieve dbus reply" };
    }
    dbus_pending_call_unref( pending );

    // read the parameters
    if ( !dbus_message_iter_init( msg, &args ) )
    {
        throw std::runtime_error { "returned message has no arguments" };
    }

    if ( DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type( &args ) )
    {
        throw std::runtime_error { "return value is not a boolean" };
    }

    dbus_message_iter_get_basic( &args, &result );

    dbus_message_unref( msg );

    // shared dbus connections must not be closed

    return result;
}

} // namespace

int main( int argc, char** argv )
{
    try
    {
        std::string msg {
            IsDesktopLocked() ? "locked" : "active"
        };
        std::cout << "Unity session is " << msg << std::endl;
    }
    catch ( std::exception& e )
    {
        std::cerr << "error: " << e.what() << std::endl;
    }

    return 0;
}
