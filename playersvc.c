/*! \file playersvc.c **********************************************************
 *
 * \brief PlayerSvc Hauptanwendung
 *
 * \author		Arndt Kritzner, Logic Way GmbH
 * \version		30. Dezember 2020 Initialversion
 * 
 *
 *****************************************************************************/

/*! \mainpage PlayerSvc
 * PlayerSvc wird durch den Oberflächen-Benutzer ("pi" auf raspberry Pi) gestartet und stellt ein Pseudo-Terminal bereit,
 * über das (Konsolen-)Drittanwendungen Medienplayer beeinflussen können.
 * <br>
 * Drittanwendungen müssen durch einen Benutzer ausgeführt werden, der der Gruppe "playerctl" angehört.
 * \attention Gruppe "playerctl" muß existieren.
 * <br>
 * Vorder-/Hintergrundausführung und Loglevel sind über Parameter beinflußbar (playersvc ?).
 * <br>
 * Das aktuell für Verbindungen ansprechbare Terminal ist aus /var/run/playersvc.pts abfragbar.
 * <br>
 * <br>
 * Aufruf beispielsweise:
 * \verbatim
 screen ./playersvc
 \endverbatim
 * Terminal-Aufruf beispielsweise (der Aufrufer sollte Gruppenmitglied in "playerctl" sein):
 * \verbatim
 minicom -D $(cat /var/run/playersvc.pts )
 \endverbatim
 * Zugriff ist über genau eine Terminalverbindung ansprechbar.
 */
#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <shadow.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

/*
#include <getopt.h>
*/

#include "playersvc.h"

/*!@addtogroup playersvc Zentrale Mechanismen, Hauptfunktion
@brief Logic Way Medienplayer-Controller
*/
/*!@{*/

typedef struct tKonsoleContext tKonsoleContext;

struct tKonsoleContext {
//    unsigned int ID;
//    eKonsoleType Type;
//    eKonsoleStatus Status;
//    tKonsoleContext *Next;
    char *Name;
    int FileDesc;
    char line_buf[CONSOLE_LINE_BUF_LEN];
    char *line_feed;
    ssize_t line_buf_len;
    char *cmd_str, *arg_str;
    char par_str1[128], par_str2[128];
    bool cmd;
    uint8_t cmd_type;
//    void *lock;
};

#define LOG_MESSAGE_LEN 256
#define THREAD_INTERVAL_uSECONDS 100000L
#define CHECK_CMD(cmdline, cmdstr, cmd, cmdlen, result)	(!cmdcmp(cmdline, cmdstr, &cmdlen)) result = cmd

static unsigned char LL = 10;
static pthread_mutex_t m_log=PTHREAD_MUTEX_INITIALIZER;
volatile bool cmd_finish = false;
static char SearchPaths[CONFIG_PATHS_LENGTH] = "\0";

void Log ( unsigned char L, char *format, ... )
{
        va_list args;
        if ( L <= LL ) {
                va_start ( args, format );
                pthread_mutex_lock ( &m_log );
                vprintf ( format, args );
                printf ( "\n" );
                pthread_mutex_unlock ( &m_log );
                va_end ( args );
        }
}

int os_run ( int result_out, char *format, ... ) {
    char command[CONSOLE_LINE_BUF_LEN + 1];
    int command_len, return_len;
    FILE *command_;
    char command_return[CONSOLE_LINE_BUF_LEN + 1];
    va_list args;
    va_start ( args, format );
    command_len = vsnprintf(command, CONSOLE_LINE_BUF_LEN + 1, format, args );
    va_end ( args );    
    if ((command_len > 0) && (command_len <= CONSOLE_LINE_BUF_LEN)) {
        if (command[command_len-1] == '&') {
            system(command);
        } else {
            command_ = popen(command, "r");
            if (command_) {
                while (fgets(command_return, CONSOLE_LINE_BUF_LEN, command_) != NULL) {
                    if (result_out) {
                        return_len = strlen(command_return);
                        while ((return_len > 0) && ((command_return[return_len-1] == '\r') || (command_return[return_len-1] == '\n'))) return_len--;
                        if (return_len > 0) {
                            write(result_out, command_return, return_len);
                            write(result_out, CONSOLE_LF, strlen(CONSOLE_LF));
                        }
                    }
                }
                pclose(command_);
                return 0;
            } else {
                Log(0, "opening pipe to execute \"%s\" failed", command);
            }
        }
    } else {
        Log(0, "line buffer to short (%d characters) to hold command string", CONSOLE_LINE_BUF_LEN);
    }
    return -1;
}

/// Konsolen-Device-Runfile (ähnlich wie PID-File) anlegen
static int konsole_publish ( tKonsoleContext *Konsole ) {
    FILE * file;
    if (Konsole && Konsole->Name) {
        file = fopen (CONSOLE_RUN_FILE, "w+");
        if (file) {
            fputs(Konsole->Name, file);
            fclose(file);
            return 0;
        } else {
            Log(0, "unable to publish pseudo terminal %s (%s)", Konsole->Name, strerror(errno));
            os_run( 0, "echo '%s' | sudo -n /usr/bin/tee %s >/dev/null", Konsole->Name, CONSOLE_RUN_FILE );
        }
        // anschließend (beispielsweise) möglich: minicom -D $(cat /var/run/playersvc.pts)
    }
    return -1;
}

/// Konsolen-Device-Runfile (ähnlich wie PID-File) löschen
static int konsole_unpublish (void) {
    if (remove(CONSOLE_RUN_FILE) == 0) {
        return 0;
    } else {
        Log(0, "unable to remove pseudo terminal %s (%s)", CONSOLE_RUN_FILE, strerror(errno));
        os_run( 0, "sudo -n /usr/bin/rm %s", CONSOLE_RUN_FILE );
    }
    return -1;
}

bool check_finish ( void )
{
    return cmd_finish;
}

void set_finish ( void )
{
    cmd_finish = true;
}

/// Interrupt-Handler
static void sig_INT(int sig_nr) {
    Log(0, "sigint received");
    set_finish();
}

// Konsolen-Eingaben verarbeiten ======================================================================================
/// Zeilenumbrüche durch '\0'-Zeichen ersetzen
static bool line_split( char * line_buf, ssize_t line_buf_len )
{
    bool result = false;
    for (ssize_t idx=0; idx<line_buf_len; idx++) {
        switch (line_buf[idx]) {
            case '\r':
            case '\n':
            case '\0':
                line_buf[idx] = '\0';
                result = true;
                break;
            default:
                break;
        }
    }
    return result;
}

/// eine Zeile aus dem Zeilenpuffer entfernen
static bool line_step( char * line_buf, ssize_t * line_buf_len )
{
    bool result = false;
    bool last_char_delimiter = false;
    for (ssize_t idx=0; idx<*line_buf_len; idx++) {
        switch (line_buf[idx]) {
            case '\0':
                if (!last_char_delimiter)
                    last_char_delimiter = true;
                break;
            default:
                if (last_char_delimiter) {
                    memmove(line_buf, line_buf+idx, *line_buf_len-idx);
                    *line_buf_len -= idx;
                    return true;
                }
                break;
        }
    }
    if (last_char_delimiter) {
        *line_buf_len = 0;
        return true;
    }
    return result;
}

/// Ersetzung undruckbarer Zeichen (durch '.')
static bool line_printable( const char * line_buf, const ssize_t line_buf_len, char * print_line_buf, const ssize_t print_line_buf_len )
{
    bool result = false;
    bool closed = false;
    ssize_t print_line_buf_pos = 0;
    for (ssize_t idx = 0; idx < line_buf_len; idx++) {
        if (print_line_buf_pos >= print_line_buf_len) {
            print_line_buf[print_line_buf_len - 1] = '\0';
            closed = true;
            break;
        }
        if ((line_buf[idx] == '\0') || (line_buf[idx] >= ' ')) {
            print_line_buf[print_line_buf_pos] = line_buf[idx];
            print_line_buf_pos++;
        } else {
            print_line_buf[print_line_buf_pos] = '.';
            print_line_buf_pos++;
            result = true;
        }
        if (line_buf[idx] == '\0') {
            closed = true;
            break;
        }
    }
    if (!closed) {
        if (print_line_buf_pos < print_line_buf_len) {
            print_line_buf[print_line_buf_pos] = '\0';
        } else {
            print_line_buf[print_line_buf_len - 1] = '\0';
        }
    }
    return result;
}

static void paramsplit ( char * const cmdline, uint8_t position, char * const param1, char * const param2, char ** const args )
{
    uint8_t paramstart[3] = {0, 0, 0}, paramlen[3] = {0, 0, 0};
    int paramidx = 0, paramstatus = -1;

    paramstatus = -1;
    for ( ; position <= strlen ( cmdline ); position++ ) {
        switch ( * ( cmdline + position ) ) {
        case ' ':
        case '\t':
            switch ( paramstatus ) {
            case -1:		// Initialstatus, Leerzeichen erwartet
                paramstatus = 0;
                break;
            case 0:
                break;		// vor Parameterinhalt
            case 1:
                paramstatus = 0;	// Parameterende
                paramlen[paramidx] = position - paramstart[paramidx];
                paramidx++;
                break;
            case 2:
                break;		// Leerzeichen in "..."
            }
            break;
        case '"':
            switch ( paramstatus ) {
            case -1:
            case 0:
                paramstatus = 2;	// Gänsefüßchen am Parameteranfang
                paramstart[paramidx] = position+1;
                break;
            case 1:
                break;		// Gänsefüßchen im Parameter
            case 2:
                paramstatus = 0;	// Gänsefüßchen am Parameterende
                paramlen[paramidx] = position - paramstart[paramidx];
                paramidx++;
                break;
            }
            break;
        case '\0':
            switch ( paramstatus ) {
            case 1:
                paramstatus = 0;	// offener Parameter
                paramlen[paramidx] = position - paramstart[paramidx];
                paramidx++;
                break;
            default:
                break;		// kein nutzbarer Parameter
            }
            break;
        default:
            switch ( paramstatus ) {
            case -1:		// fehlender Zwischenraum zwischen Befehl und Parameter
                paramstatus = -2;
                break;
            case 0:
                paramstatus = 1;	// Parameteranfang ohne Gänsefüßchen
                paramstart[paramidx] = position;
                break;
            case 1:		// Parameterinhalt
            case 2:
                break;
            }
            break;
        }
        if ( paramidx >= 3 ) {
            break;
        }
        if ( paramstatus < -1 ) {
            break;
        }
    }
    if ( param1 ) {
        if ( paramlen[0] ) {
            strncpy ( param1, cmdline+paramstart[0], paramlen[0] );
        }
        * ( param1 + paramlen[0] ) = '\0';
    }
    if ( param2 ) {
        if ( paramlen[1] ) {
            strncpy ( param2, cmdline+paramstart[1], paramlen[1] );
        }
        * ( param2 + paramlen[1] ) = '\0';
    }
    if ( args ) {
        *args = NULL;
        if ( paramlen[2] ) {
            *args = cmdline+paramstart[2];
        }
    }
}

int cmdcmp ( char const * const cmdline, char * const cmdstr, int * const cmdlen )
{
    char *cmdtest = cmdstr, *cmd_delim;
    char *cmddelimiter = "|";
    int result = -1;
    while ( cmdtest ) {
        cmd_delim = strstr ( cmdtest, cmddelimiter );
        if ( ! ( result = strncmp ( cmdline, cmdtest, ( cmd_delim ? cmd_delim-cmdtest : strlen ( cmdtest ) ) ) ) ) {
            if ( cmdlen ) {
                *cmdlen = ( cmd_delim ? cmd_delim-cmdtest : strlen ( cmdtest ) );
            }
            break;
        }
        cmdtest = ( cmd_delim ? cmd_delim + strlen ( cmddelimiter ) : cmd_delim );
    }
    return result;
}

static void konsole_print( tKonsoleContext *Ctx, char * output )
{
    write(Ctx->FileDesc, output, strlen(output));
    // hmn sync ausführen
}

static uint8_t parse_cmd ( char *cmdline, tKonsoleContext *KonsoleCtx )
{

    uint8_t result = CMD_NONE;
    int position = 0;
    // Decode command type.
    if	CHECK_CMD ( cmdline, STR_EXIT, CMD_EXIT, position, result );
    if	CHECK_CMD ( cmdline, STR_ECHO, CMD_ECHO, position, result );
#define SPECIFIC_COMMANDS_PARSE
// ====================================================================================================================
#include "specific-commands.h"
#undef SPECIFIC_COMMANDS_PARSE
    else {
        // error : command not found
        if ( strlen ( cmdline ) ) {
            konsole_print ( KonsoleCtx, MSG_ER_CMD_NOT_FOUND );
            Log(0, "undefined command: %s", cmdline );
        }
    }
    // if command isn't found, display prompt
    if ( result == CMD_NONE ) {
        return result;
    }
    KonsoleCtx->par_str1[0] = '\0';
    KonsoleCtx->par_str2[0] = '\0';
    paramsplit ( cmdline, position, KonsoleCtx->par_str1, KonsoleCtx->par_str2, &(KonsoleCtx->arg_str) );
    return result;
}

bool konsole_run ( tKonsoleContext *Ctx )
{
    ssize_t read_len;
    char print_buf[CONSOLE_LINE_BUF_LEN];
    bool Result = false;
    if ( !(Ctx->cmd) ) {
        read_len = read(Ctx->FileDesc, &(Ctx->line_buf[Ctx->line_buf_len]), sizeof(Ctx->line_buf)-1-Ctx->line_buf_len);
        if (read_len > 0) {
            Ctx->line_buf_len += read_len;
            Ctx->line_buf[Ctx->line_buf_len] = '\0';
            line_printable(Ctx->line_buf, Ctx->line_buf_len, print_buf, sizeof(print_buf));
            Log(20, "read %d bytes: |%s|", read_len, print_buf);
        }
        if (line_split(Ctx->line_buf, Ctx->line_buf_len)) {
            line_printable(Ctx->line_buf, Ctx->line_buf_len, print_buf, sizeof(print_buf));
            Log(10, "command line: %s", (strlen(Ctx->line_buf) ?  Ctx->line_buf : "<empty>"));
            if ( ( strlen ( Ctx->line_buf ) < sizeof ( Ctx->par_str1 ) ) && ( strlen ( Ctx->line_buf ) < sizeof ( Ctx->par_str2 ) ) ) {
                if ( ( Ctx->cmd_type = parse_cmd ( Ctx->line_buf, Ctx ) ) != CMD_NONE ) {
                    Ctx->cmd = true;
                }
                /// \todo hier evtl. auch Antwort auf unbekannte Kommandos spendieren
                Result = true;
            }
            line_step(Ctx->line_buf, &(Ctx->line_buf_len));
        } else {
            if (Ctx->line_buf_len >= sizeof(Ctx->line_buf) - 1) {
                Log(0, "console input line overflow -> throwing");
                Ctx->line_buf_len = 0;
            }
        }
    } else {
        Log ( 19, "cmd_type: %d\n", Ctx->cmd_type );
        switch ( Ctx->cmd_type ) {
            // exit command: console instance finish
            case CMD_EXIT:
                konsole_print ( Ctx, MSG_EXIT );
                set_finish();
                break;
            case CMD_ECHO:
                break;
#define SPECIFIC_COMMANDS_RUN
// ====================================================================================================================
#include "specific-commands.h"
#undef SPECIFIC_COMMANDS_RUN
            // Unknown command.
            default:
                // Display error message.
                Log(0, "command (%s) recognized, but no action defined", Ctx->cmd_str);
                break;
        }
        // Reset vars.
        Ctx->cmd_type = CMD_NONE;
        Ctx->cmd = false;
        
        Result = true;
    }
    return Result;
}

#define SPECIFIC_COMMANDS_FUNCTIONS
// ====================================================================================================================
#include "specific-commands.h"
#undef SPECIFIC_COMMANDS_FUNCTIONS

int main (int argc, char **argv) {
    char Name[128] = "\0";
    struct stat ContextStat;
    struct group *ContextGroup;
    tKonsoleContext *Context = malloc(sizeof(tKonsoleContext));
    if (!Context)
        return -1;
    signal(SIGINT, sig_INT);
    Log(10, "program %s started", argv[0]);
    if ( !ReadConfigPaths( CONFIG_FILE, CONFIG_PATHS_LENGTH, SearchPaths ) ) {
        return -7;
    }
    // reset vars
    Context->cmd = false;
    Context->cmd_type = CMD_NONE;
    Context->cmd_str = NULL;
    Context->line_buf[0] = '\0';
    Context->line_buf_len = 0;
    Context->line_feed=CONSOLE_LF;
    Context->FileDesc = posix_openpt(O_RDWR | O_NONBLOCK);
    if (Context->FileDesc < 0) {
        Log(0, "Unable to create posix pseudo terminal");
        cmd_finish = true;
        free(Context);
        return -2;
    }
    if ((grantpt(Context->FileDesc) != 0) || (unlockpt(Context->FileDesc) != 0)) {
        Log(0, "Error %d in posix pseudo terminal grant/ unlock", errno);
        cmd_finish = true;
        free(Context);
        return -3;
    }
    // Terminalfile für Gruppe les- und schreibbar machen, Gruppenzugehörigkeit auf "playersvc" (TERMINAL_GROUP) setzen
    if (fstat(Context->FileDesc, &ContextStat) == 0) {
        ContextStat.st_mode |= (S_IRGRP | S_IWGRP);
        if (chmod(ptsname(Context->FileDesc), ContextStat.st_mode) == 0) { // fchmod ist auf offenen Filedescriptor nicht anwendbar
            Log(5, "posix pseudo terminal chmod set to 0o%o", ContextStat.st_mode);
        } else {
            Log(0, "Error %d in posix pseudo terminal chmod", errno);
            cmd_finish = true;
            free(Context);
            return -4;
        }
        ContextGroup = getgrnam(TERMINAL_GROUP);
        if (ContextGroup && (chown(ptsname(Context->FileDesc), ContextStat.st_uid, ContextGroup->gr_gid) == 0)) {
            Log(5, "posix pseudo terminal group ownership set to %s", ContextGroup->gr_name);
        }
    } else {
        Log(0, "Error %d in posix pseudo terminal permissions gathering", errno);
        cmd_finish = true;
        free(Context);
        return -5;
    }
    // (nochmal) Filenamen für Pseudoterminal-Gerätefile ermitteln und eintragen
    strncpy(Name, ptsname(Context->FileDesc), sizeof(Name)-1);
    if (strlen(Name)) {
        Context->Name = malloc(strlen(Name)+1);
        if (Context->Name) {
            strncpy(Context->Name, Name, strlen(Name)+1);
            konsole_publish(Context);
            Log(5, "konsole started on %s", Context->Name);
        } else {
            Log(0, "Unable to create konsole context");
            cmd_finish = true;
            free(Context);
            return -6;
        }
    }
    // Programm-Hauptschleife
    for (;;) {
        if (check_finish()) break;
        if (konsole_run(Context)) {
            if (Context->cmd) {
                Log(10, "command: %s", Context->line_buf);
            }
        } else {
            usleep(THREAD_INTERVAL_uSECONDS);
        }
    }
    // Abschluss, Speicher- und Filefreigabe
    konsole_unpublish();
    if ( Context->cmd_str ) {
        free ( Context->cmd_str );
        Context->cmd_str = NULL;
    }
    if (Context->FileDesc > 0) {
        close(Context->FileDesc);
        Context->FileDesc = 0;
    }
    if (Context->Name) {
        if (unlink(Context->Name) == 0) {
            Log(10, "gelöscht: %s", Context->Name);
        }
    }
    Log(5, "%s: konsole and program closed: %s", argv[0], (Context->Name ? Context->Name : "?"));
    free(Context);
}

/*!@}*/
