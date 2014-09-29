#include "winsystemcommand.h"
#include <windows.h>
#include <QObject>

namespace WinSystemCommand
{
    // Функция для выполнение команд, требущий определенных привилегий
    bool systemControl( Command command )
    {
        HANDLE hToken; // указатель на идентификатор процесса
        TOKEN_PRIVILEGES tkp; // указатель на идентифицирующую структуру
        // получение идентификатора текущего процесса, для получения прав на отключение
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
            return FALSE;

        // Полчение LUID
        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1; // установка одной привилегии
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        //Установка привилегии отключения для этого процесса.
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,(PTOKEN_PRIVILEGES) NULL, 0);

        // Если ошибка то выход.
        if (GetLastError() != ERROR_SUCCESS)
            return FALSE;

        bool successful = false;

        // Ниже используются функции, требующие определенных привилегий
        switch ( command )
        {
        case PowerOff:
            successful = ExitWindowsEx( EWX_POWEROFF, 0 ); // выключение питания компьютера
            break;

        case Reboot:
            successful = ExitWindowsEx( EWX_REBOOT, 0 ); // перезагрузка
            break;

        case Shutdown:
            successful = ExitWindowsEx( EWX_SHUTDOWN, 0 ); // завершение сеанса
            break;

        case Hibernate:
            successful = SetSystemPowerState( FALSE, FALSE ); // cпящий режим
            break;

        case Suspend:
            successful = SetSystemPowerState( TRUE, FALSE ); // ждущий режим
            break;
        }

        // Отменить привилегии отключения.
        tkp.Privileges[0].Attributes = 0;
        AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

        return successful;
    }

    bool execute( Command command )
    {
        bool successful = true;

        switch ( command )
        {
        case Force:
            successful = ExitWindowsEx( EWX_FORCE, 0 );
            break;

        case PowerOff:
        case Reboot:
        case Shutdown:
        case Hibernate:
        case Suspend:
            successful = systemControl( command );
            break;

        case LogOff:
            successful = ExitWindowsEx( EWX_LOGOFF, 0 );
            break;

        case ScreenOff:
            SendMessageW( HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2 );
            successful = true;
            break;
        }

        return successful;
    }

    QString getTextError( Command command )
    {
        QString message;

        switch ( command )
        {
        case Force:
            message = QObject::tr( "Unable to turn off the computer" ); // Не удалось выключить питание компьютера
            break;

        case PowerOff:
            message = QObject::tr( "Unable to turn off your computer" ); // Не удалось выключить компьютер
            break;

        case Reboot:
            message = QObject::tr( "Unable to restart the computer" ); // Не удалось перезагрузить компьютер
            break;

        case Shutdown:
            message = QObject::tr( "Failed to complete the session" ); // Не удалось завершить сеанс
            break;

        case Hibernate:
            message = QObject::tr( "Unable to go to sleep" ); // Не удалось перейти в режим сна
            break;

        case Suspend:
            message = QObject::tr( "Unable to switch to standby" ); // Не удалось перейти в режим ожидания
            break;

        case LogOff:
            message = QObject::tr( "Failed to change the user" ); // Не удалось сменить пользователя
            break;
        }

        return message;
    }
}
