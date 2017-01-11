#ifndef SRC_DBUSERROR_HPP_
#define SRC_DBUSERROR_HPP_

#include <dbus/dbus.h>
#include <string>

namespace ult
{
class DBusError
{
public:
    DBusError()
    {
    }

    ~DBusError()
    {
        FreeError();
    }

    operator ::DBusError*()
    {
        FreeError();
        return &error_;
    }

    bool IsError() const
    {
        return dbus_error_is_set( &error_ );
    }

    std::string GetMessage() const
    {
        if( dbus_error_is_set( &error_ ) )
        {
            return error_.message;
        }

        return "";
    }

private:
    DBusError( const DBusError& ) = delete;

    void FreeError()
    {
        if( dbus_error_is_set( &error_ ) )
        {
            dbus_error_free( &error_ );
        }
    }

private:
    ::DBusError error_;
};

} // namespace


#endif /* SRC_DBUSERROR_HPP_ */
