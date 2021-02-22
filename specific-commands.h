// (C) Arndt Kritzner, 2021

/// \todo hier weitere Kommandos (info)
#if defined(SPECIFIC_COMMANDS_DEFINITIONS)
/// \todo hier zusätzliche Kommando-Definitionen (wie in playersvc.h)
/// \attention in playersvc.h bereits definierte Kommandos nicht doppelt vergeben (nicht als ID und nicht als Kommando)
#define CMD_HELP		0x9
#define CMD_PLAY		0x10
#define CMD_DISPLAY		0x11
#define CMD_OPEN		0x12
#define CMD_ENQUEUE		0x13
#define CMD_CLOSE		0x14
#define CMD_PATHS		0x15
#define STR_HELP		"help|?"
#define STR_PLAY		"playerctl|play"
#define STR_DISPLAY		"display|disp"
#define STR_OPEN		"open"
#define STR_ENQUEUE		"enqueue|enq"
#define STR_CLOSE		"close"
#define STR_PATHS		"paths|path"

#define MSG_OK			"ok"CONSOLE_LF
#define MSG_FAILED		"fail"CONSOLE_LF
#define MSG_OK_DATA_START	"ok >>>"CONSOLE_LF
#define MSG_DATA_END		"<<<"CONSOLE_LF

#define STR_FOTO		"foto|photo"
#define STR_AUDIO		"audio"
#define STR_VIDEO		"video"
#define STR_SLIDESHOW		"slideshow|slides"
#define STR_LIST		"list"
#define STR_STATUS		"status"
#define STR_METADATA		"metadata"
#define STR_ALLSTATUS		"allstatus"
#define STR_START		"start|play"
#define STR_STOP		"stop"
#define STR_PAUSE		"pause"
#define STR_PREVIOUS		"previous|prev"
#define STR_NEXT		"next"
#define STR_ON			"on"
#define STR_OFF			"off"
#define PARAM_NONE		0x0
#define PARAM_FOTO		0x1
#define PARAM_AUDIO		0x2
#define PARAM_VIDEO		0x3
#define PARAM_SLIDESHOW		0x4
#define PARAM_LIST		0x5
#define PARAM_STATUS		0x6
#define PARAM_METADATA		0x7
#define PARAM_ALLSTATUS		0x8
#define ACTION_NONE		0x0
#define ACTION_START		0x1
#define ACTION_STOP		0x2
#define ACTION_PAUSE		0x3
#define ACTION_PREVIOUS		0x4
#define ACTION_NEXT		0x5
#define ACTION_ON		0x6
#define ACTION_OFF		0x7
#define SLIDESHOW_PID		"ps -FA | grep 'SLIDESHOW' | grep -v 'grep' | awk '{print $2}'"
#define EXIT_SLIDESHOW		"if [ -f /tmp/SLIDESHOW ]; then SLIDESHOW=$("SLIDESHOW_PID"); if [ -n \"${SLIDESHOW}\" ]; then kill \"${SLIDESHOW}\"; fi; rm /tmp/SLIDESHOW; fi"
#define EXIT_FOTO		"killall gwenview"
#define EXIT_AUDIO		"killall smplayer"
#define EXIT_VIDEO		"killall vlc"
#define COMMAND_FOTO		EXIT_SLIDESHOW"; "EXIT_FOTO"; gwenview -f '%s' &"
#define COMMAND_AUDIO		"smplayer '%s' &"
#define COMMAND_AUDIO_ENQUEUE	"smplayer -add-to-playlist '%s' &"
#define COMMAND_VIDEO		"vlc -f '%s' &"
#define COMMAND_ENTER		"(sleep 2; xte -x \":0.0\" 'key Return') &"
#define PLAY_SLIDESHOW		"( if [ ! -f /tmp/SLIDESHOW ]; then touch /tmp/SLIDESHOW; while true; do playerctl %s next; sleep %s; done; fi ) &"
#define PLAY_PLAYERCTL		"playerctl %s %s"
#define PLAY_ALLSTATUS		"playerctl --list-all; echo '---'; playerctl --all-players status; echo '---'; playerctl --all-players metadata"
#define COMMAND_DISPLAY		"vcgencmd display_power %s"
#define CLOSE_FOTO		EXIT_SLIDESHOW"; "EXIT_FOTO
#define CLOSE_AUDIO		EXIT_AUDIO
#define CLOSE_VIDEO		EXIT_VIDEO

#define MSG_HELP		"Commands summary"CONSOLE_LF \
				" "STR_EXIT""CONSOLE_LF \
				" "STR_HELP""CONSOLE_LF \
				" "STR_OPEN" "STR_FOTO"|"STR_AUDIO"|"STR_VIDEO" <FILE>"CONSOLE_LF \
				" "STR_ENQUEUE" "STR_AUDIO" <FILE>"CONSOLE_LF \
				" "STR_CLOSE" "STR_FOTO"|"STR_AUDIO"|"STR_VIDEO""CONSOLE_LF \
				" "STR_PLAY" "STR_FOTO"|"STR_AUDIO"|"STR_VIDEO" "STR_START"|"STR_STOP"|"STR_PAUSE"|"STR_PLAY"|"STR_PREVIOUS"|"STR_NEXT""CONSOLE_LF \
				" "STR_PLAY" "STR_LIST""CONSOLE_LF \
				" "STR_PLAY" "STR_STATUS""CONSOLE_LF \
				" "STR_PLAY" "STR_METADATA""CONSOLE_LF \
				" "STR_PLAY" "STR_ALLSTATUS""CONSOLE_LF \
				" "STR_PLAY" "STR_SLIDESHOW"[ "STR_STOP"]"CONSOLE_LF \
				" "STR_DISPLAY" "STR_ON"|"STR_OFF""CONSOLE_LF \
				" "STR_PATHS""CONSOLE_LF \

char *UrlDecode( const char *Encoded, char *Decode );
char *ClipPrefix( char *Path );
bool IsInPaths( char *File, char *Paths );
#endif

#if defined(SPECIFIC_COMMANDS_PARSE)
/// \todo hier zusätzliche Kommando-Erkennungen (wie in playersvc.c)
    else if	CHECK_CMD ( cmdline, STR_HELP, CMD_HELP, position, result );
    else if	CHECK_CMD ( cmdline, STR_PLAY, CMD_PLAY, position, result );
    else if	CHECK_CMD ( cmdline, STR_DISPLAY, CMD_DISPLAY, position, result );
    else if	CHECK_CMD ( cmdline, STR_OPEN, CMD_OPEN, position, result );
    else if	CHECK_CMD ( cmdline, STR_ENQUEUE, CMD_ENQUEUE, position, result );
    else if	CHECK_CMD ( cmdline, STR_CLOSE, CMD_CLOSE, position, result );
    else if	CHECK_CMD ( cmdline, STR_PATHS, CMD_PATHS, position, result );
#endif

#if defined(SPECIFIC_COMMANDS_RUN)
/// \todo hier zusätzliche Kommando-Ausführung (wie in playersvc.c)
            case CMD_HELP:
                Log(10, "HELP: %s %s", Ctx->par_str1 ? Ctx->par_str1 : "NULL", Ctx->par_str2 ? Ctx->par_str2 : "NULL" );
                {
                    konsole_print ( Ctx, MSG_OK_DATA_START );
                    konsole_print ( Ctx, MSG_HELP );
                    konsole_print ( Ctx, MSG_DATA_END );
                }
                break;
            case CMD_PLAY:
                Log(10, "PLAY: %s %s", Ctx->par_str1 ? Ctx->par_str1 : "NULL", Ctx->par_str2 ? Ctx->par_str2 : "NULL" );
                {
                    bool ok = false;
                    uint8_t media = PARAM_NONE, action = ACTION_NONE;
                    int position = 0;
                    char *cmd = NULL, *player = NULL, *command = NULL, *interval = "15";
                    if (Ctx->par_str1) {
                        if	CHECK_CMD ( Ctx->par_str1, STR_FOTO, PARAM_FOTO, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_AUDIO, PARAM_AUDIO, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_VIDEO, PARAM_VIDEO, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_SLIDESHOW, PARAM_SLIDESHOW, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_LIST, PARAM_LIST, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_STATUS, PARAM_STATUS, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_METADATA, PARAM_METADATA, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_ALLSTATUS, PARAM_ALLSTATUS, position, media );
                    }
                    if (Ctx->par_str2) {
                        position = 0;
                        if	CHECK_CMD ( Ctx->par_str2, STR_START, ACTION_START, position, action );
                        else if	CHECK_CMD ( Ctx->par_str2, STR_STOP, ACTION_STOP, position, action );
                        else if	CHECK_CMD ( Ctx->par_str2, STR_PAUSE, ACTION_PAUSE, position, action );
                        else if	CHECK_CMD ( Ctx->par_str2, STR_PREVIOUS, ACTION_PREVIOUS, position, action );
                        else if	CHECK_CMD ( Ctx->par_str2, STR_NEXT, ACTION_NEXT, position, action );
                        else if ((strlen(Ctx->par_str2) > 0) && (strlen(Ctx->par_str2) <= 4)) {
                            bool digits = true, value = false;
                            for (int idx = 0; idx < strlen(Ctx->par_str2); idx++) {
                                if ((Ctx->par_str2[idx] >= '0') && (Ctx->par_str2[idx] <= '9')) {
                                    if (Ctx->par_str2[idx] >= '1') value = true;
                                } else {
                                    digits = false;
                                    break;
                                }
                            }
                            if (value && digits) interval = Ctx->par_str2;
                        }
                    }
                    switch (media) {
                        case PARAM_FOTO:
                            cmd = PLAY_PLAYERCTL;
                            player = "--player=Gwenview";
                            break;
                        case PARAM_AUDIO:
                            cmd = PLAY_PLAYERCTL;
                            player = "--player=smplayer";
                            break;
                        case PARAM_VIDEO:
                            cmd = PLAY_PLAYERCTL;
                            player = "--player=vlc";
                            break;
                        case PARAM_SLIDESHOW:
                            switch (action) {
                                case ACTION_STOP:
                                    cmd = EXIT_SLIDESHOW;
                                    break;
                                default:
                                    cmd = PLAY_SLIDESHOW;
                                    player = "--player=Gwenview";
                                    break;
                            }
                            break;
                        case PARAM_LIST:
                            cmd = PLAY_PLAYERCTL;
                            command = "--list-all";
                            break;
                        case PARAM_STATUS:
                            cmd = PLAY_PLAYERCTL;
                            command = "status";
                            break;
                        case PARAM_METADATA:
                            cmd = PLAY_PLAYERCTL;
                            command = "metadata";
                            break;
                        case PARAM_ALLSTATUS:
                            cmd = PLAY_ALLSTATUS;
                            break;
                    }
                    if (cmd != NULL) {
                        Log(10, "PLAY: %d command:\"%s\" %s %s", media, cmd, player ? player : "NULL", command ? command : "NULL" );
                        switch (media) {
                            case PARAM_FOTO:
                            case PARAM_AUDIO:
                            case PARAM_VIDEO:
                                switch (action) {
                                    case ACTION_START:
                                        command = "play";
                                        break;
                                    case ACTION_STOP:
                                        command = "stop";
                                        break;
                                    case ACTION_PAUSE:
                                        command = "pause";
                                        break;
                                    case ACTION_PREVIOUS:
                                        command = "previous";
                                        break;
                                    case ACTION_NEXT:
                                        command = "next";
                                        break;
                                }
                                if (action != ACTION_NONE) {
                                    ok = true;
                                    konsole_print ( Ctx, MSG_OK );
                                    os_run ( 0, cmd, player ? player : "", command ? command : "" );
                                }
                                break;
                            case PARAM_SLIDESHOW:
                                ok = true;
                                konsole_print ( Ctx, MSG_OK );
                                switch (action) {
                                    case ACTION_STOP:
                                        os_run ( 0, cmd );
                                        break;
                                    default:
                                        os_run ( 0, cmd, player ? player : "", interval );
                                        break;
                                }
                                break;
                            case PARAM_LIST:
                            case PARAM_STATUS:
                            case PARAM_METADATA:
                                ok = true;
                                konsole_print ( Ctx, MSG_OK_DATA_START );
                                os_run ( Ctx->FileDesc, cmd, "", command ? command : "" );
                                konsole_print ( Ctx, MSG_DATA_END );
                                break;
                            case PARAM_ALLSTATUS:
                                ok = true;
                                konsole_print ( Ctx, MSG_OK_DATA_START );
                                os_run ( Ctx->FileDesc, cmd );
                                konsole_print ( Ctx, MSG_DATA_END );
                                break;
                        }
                    }
                    if (! ok) konsole_print ( Ctx, MSG_FAILED );
                }
                break;
            case CMD_DISPLAY:
                Log(10, "DISPLAY: %s %s", Ctx->par_str1 ? Ctx->par_str1 : "NULL", Ctx->par_str2 ? Ctx->par_str2 : "NULL" );
                {
                    bool ok = false;
                    uint8_t action = ACTION_NONE;
                    int position = 0;
                    char *cmd = COMMAND_DISPLAY, *command = NULL;
                    if (Ctx->par_str1) {
                        if	CHECK_CMD ( Ctx->par_str1, STR_ON, ACTION_ON, position, action );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_OFF, ACTION_OFF, position, action );
                    }
                    switch (action) {
                        case ACTION_ON:
                            command = "1";
                            break;
                        case ACTION_OFF:
                            command = "0";
                            break;
                    }
                    if (command) {
                        ok = true;
                        konsole_print ( Ctx, MSG_OK );
                        os_run ( 0, cmd, command );
                    }
                    if (! ok) konsole_print ( Ctx, MSG_FAILED );
                }
                break;
            case CMD_OPEN:
            case CMD_ENQUEUE:
                Log(10, "%s: %s %s", Ctx->cmd_type == CMD_OPEN ? "OPEN" : "ENQUEUE", Ctx->par_str1 ? Ctx->par_str1 : "NULL", Ctx->par_str2 ? Ctx->par_str2 : "NULL" );
                {
                    bool ok = false;
                    uint8_t media = PARAM_NONE;
                    int position = 0;
                    mode_t file_flags = (S_IFREG | S_IFDIR);
                    char *cmd = NULL, *cmd_pre = NULL, *cmd_post = NULL, *topic = NULL;
                    if (Ctx->par_str1) {
                        if	CHECK_CMD ( Ctx->par_str1, STR_FOTO, PARAM_FOTO, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_AUDIO, PARAM_AUDIO, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_VIDEO, PARAM_VIDEO, position, media );
                    }
                    switch (media) {
                        case PARAM_FOTO:
                            cmd = COMMAND_FOTO;
                            topic = "FOTO";
                            break;
                        case PARAM_AUDIO:
                            cmd = (Ctx->cmd_type == CMD_OPEN ? COMMAND_AUDIO : COMMAND_AUDIO_ENQUEUE);
                            file_flags = S_IFREG;
                            topic = "AUDIO";
                            break;
                        case PARAM_VIDEO:
                            cmd = COMMAND_VIDEO;
                            cmd_post = COMMAND_ENTER;
                            topic = "VIDEO";
                            break;
                    }
                    if (cmd != NULL) {
                        struct stat statbuf;
                        char *ParBuf = NULL;
                        if (Ctx->par_str2) {
                            ParBuf = malloc(strlen(Ctx->par_str2) + 1);
                            ParBuf = UrlDecode(Ctx->par_str2, ParBuf);
                        }
                        if (IsInPaths(ClipPrefix(ParBuf), SearchPaths)) {
                            stat(ClipPrefix(ParBuf), &statbuf);
                            if ((statbuf.st_mode & S_IFMT) & file_flags) {
                                ok = true;
                                konsole_print ( Ctx, MSG_OK );
                                os_run ( 0, cmd, ParBuf );
                                Log(0, cmd, ParBuf);
                                if (cmd_post != NULL) {
                                    os_run ( 0, cmd_post );
                                }
                            } else {
                                konsole_print( Ctx, ParBuf );
                                switch (file_flags) {
                                    case (S_IFREG | S_IFDIR) :
                                        konsole_print( Ctx, ": file or directory not found"CONSOLE_LF );
                                        Log(10, "OPEN %s: %s: file or directory not found", (topic != NULL ? topic : ""), ParBuf);
                                        break;
                                    case S_IFREG :
                                        konsole_print( Ctx, ": file not found"CONSOLE_LF );
                                        Log(10, "OPEN %s: %s: file not found", (topic != NULL ? topic : ""), ParBuf);
                                        break;
                                }
                            }
                        } else {
                            konsole_print( Ctx, ParBuf );
                            konsole_print( Ctx, ": file doesn't match configured pattern"CONSOLE_LF );
                            Log(10, "OPEN %s: %s: file doesn't match configured pattern", (topic != NULL ? topic : ""), ParBuf);
                        }
                        if (ParBuf) free(ParBuf);
                    }
                    if (! ok) konsole_print ( Ctx, MSG_FAILED );
                }
                break;
            case CMD_CLOSE:
                Log(10, "CLOSE: %s", Ctx->par_str1 ? Ctx->par_str1 : "NULL" );
                {
                    bool ok = false;
                    uint8_t media = PARAM_NONE;
                    int position = 0;
                    char *cmd = NULL;
                    if (Ctx->par_str1) {
                        if	CHECK_CMD ( Ctx->par_str1, STR_FOTO, PARAM_FOTO, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_AUDIO, PARAM_AUDIO, position, media );
                        else if	CHECK_CMD ( Ctx->par_str1, STR_VIDEO, PARAM_VIDEO, position, media );
                    }
                    switch (media) {
                        case PARAM_FOTO:
                            cmd = CLOSE_FOTO;
                            break;
                        case PARAM_AUDIO:
                            cmd = CLOSE_AUDIO;
                            break;
                        case PARAM_VIDEO:
                            cmd = CLOSE_VIDEO;
                            break;
                    }
                    if (cmd != NULL) {
                        ok = true;
                        konsole_print ( Ctx, MSG_OK );
                        os_run ( 0, cmd );
                    }
                    if (! ok) konsole_print ( Ctx, MSG_FAILED );
                }
                break;
            case CMD_PATHS:
                Log(10, "PATHS: %s", SearchPaths );
                {
                    konsole_print ( Ctx, MSG_OK_DATA_START );
                    konsole_print ( Ctx, SearchPaths );
                    konsole_print ( Ctx, CONSOLE_LF );
                    konsole_print ( Ctx, MSG_DATA_END );
                }
                break;
#endif

#if defined SPECIFIC_COMMANDS_FUNCTIONS
char *UrlDecode( const char *Encoded, char *Decode ) {
    char *replace[][2] = {{"%20", " "}, {"%25", "%"}, {"%26", "&"}, {"%28", "("}, {"%29", ")"}, };
    if (Decode != NULL) {
        strcpy(Decode, "\0");
    }
    if ((Encoded != NULL) && (Decode != NULL)) {
        size_t offset = 0;
        while (offset < strlen(Encoded)) {
            bool replaced = false;
            for (int idx = 0; idx < sizeof(replace)/sizeof(replace[0]); idx++) {
                if (strncmp(Encoded + offset, replace[idx][0], strlen(replace[idx][0])) == 0) {
                    strcpy(Decode + strlen(Decode), replace[idx][1]);
                    offset += strlen(replace[idx][0]);
                    replaced = true;
                    break;
                }
            }
            if (!replaced) {
                Decode[strlen(Decode) + 1] = '\0';
                Decode[strlen(Decode)] = Encoded[offset];
                offset++;
            }
        }
    }
    return Decode;
}

char *ClipPrefix( char *Path ) {
    char *prefixes[] = { "dvd:/", };
    char *result = Path;
    for (int idx = 0; idx < sizeof(prefixes)/sizeof(prefixes[0]); idx++) {
        if (strncmp(result, prefixes[idx], strlen(prefixes[idx])) == 0) {
            result += strlen(prefixes[idx]);
            break;
        }
    }
    return result;
}

bool IsInPaths( char *File, char *Paths ) {
    bool result = false;
    size_t offset = 0;
    if ((File != NULL) && (Paths != NULL)) {
        if (strstr(File, "/.") == NULL) { // suppress ".." and hidden elements
            while ((!result) && (offset < strlen(Paths))) {
                char *position = strstr(Paths + offset, ":");
                if (strncmp(Paths + offset, File, (position != NULL ? position - (Paths + offset) : strlen(Paths + offset))) == 0) {
                    result = true;
                }
                offset += (position != NULL ? position - (Paths + offset) + strlen(":") : strlen(Paths + offset));
            }
        }
    }
    return result;
}

bool ReadConfigPaths( char *ConfigFile, size_t BufSize, char *Paths ) {
    FILE *f;
    size_t offset = 0;
    f = fopen(ConfigFile, "r");
    if (f == NULL) {
        Log(0, "ReadConfigPaths: Failed to open %s (%s)\n", ConfigFile, strerror(errno));
        return false;
    }
    while (fgets(&(Paths[offset]), BufSize-offset, f)!=NULL) {
        if (Paths[offset] != '/') { 
            Paths[offset] = '\0';
        } else {
            offset=strlen(Paths);
            while ((offset>0) && ((Paths[offset - 1] == '\n') || (Paths[offset - 1] == '\r'))) {
                offset--;
                Paths[offset] = '\0';
            }
            if (offset + 1 < BufSize) {
                Paths[offset] = ':';
                offset++;
                Paths[offset] = '\0';
            }
        }
    }
    fclose(f);
    while ((offset > 0) && (Paths[offset - 1] == ':')) {
        offset--;
        Paths[offset] = '\0';
    }
    Log(0, "ReadConfigPaths: media path is %s\n", Paths);
    return true;
}
#endif
