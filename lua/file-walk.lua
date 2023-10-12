-- (C) Arndt Kritzner, 2021,2022

local _M = {}

-- echo 'local file_walk = require "lua/file-walk"; local result, err = file_walk.ls("ok >>>\r\n/misc/kritzner-nas/Fotos/:/misc/kritzner-nas/Film/:/misc/kritzner-nas/Musik/\r\n<<<", "ls /misc/kritzner-nas/Fotos/esg/ 100,100"); print( result, err )' | lua

local hex_to_char = function(x)
  return string.char(tonumber(x, 16))
end

local unescape = function(url)
  return url:gsub("%%(%x%x)", hex_to_char)
end

function _M.split(inputstr, sep)
    if sep == nil then
        sep = "%s"
    end
    local result={}
    for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
        table.insert(result, str)
    end
    return result
end

function _M.ls(paths, command)
    local result = ''
    local path = {}
    if paths ~= nil then
        local paths_line = string.match(paths, '>>>[\r\n]+([^\r\n]+)[\r\n]+<<<')
        if paths_line ~= nil then
            paths = paths_line
        end
        path = _M.split(paths, ':')
    end
    if #path > 0 then
        local rooted = false;
        local command_path, command_offset = nil, nil
        if command ~= nil then
            command_path, command_offset = string.match(command, '^ls (/.+/) *([%d,]*)$')
            if command_path ~= nil then
                command_path = unescape(command_path)   -- decode URL-encoded characters
                for k, v in ipairs(path) do
                    if string.sub(command_path, 1, string.len(v)) == v then
                        rooted = true
                        break
                    end
                end
            end
        end
        if rooted then
            local lfs = require 'lfs'
            local filename
            local filenames = {}
            local directories = {}
            local files = {}
            local result_window = { 0, 100 }
            for filename in lfs.dir(command_path) do
                table.insert(filenames, filename)
            end
            table.sort(filenames)
            for k, v in ipairs(filenames) do
                local mode = lfs.attributes(command_path .. v,"mode")
                if mode == "directory" then
                    if string.match(v, '^%.+') == nil then     -- filter out ".", ".." and all ".xxxx"
                        table.insert(directories, v .. '/')
                    end
                else
                    for k, ext in ipairs({ "jpg", "jpeg", "png", "mp3", "m3u", "mka", "mp4", "mpg", "mov", "webm", "mkv", "flv", "vob" }) do
                        if string.match(string.lower(v), '%.' .. ext .. '$') then
                            table.insert(files, v)
                            break
                        end
                    end
                end
            end
            local i = 0
            for v in string.gmatch(command_offset, '%d+') do
                i = i + 1
                result_window[i] = tonumber(v)
            end
            if result_window[2] > #directories + #files then
                result_window[2] = #directories + #files
            end
            if result_window[1] + result_window[2] > #directories + #files then
                result_window[1] = #directories + #files - result_window[2]
            end
            result = 'ok >>>\r\n'
            result = result .. command_path .. '\r\n' .. #directories .. ',' .. #files .. '==' .. result_window[1] .. ',' .. result_window[2] .. '\r\n'
            if result_window[2] > 0 then
                for k = 1, result_window[2] do
                    if result_window[1] + k > #directories then
                        result = result .. files[result_window[1] + k - #directories] .. '\r\n'
                    else
                        result = result .. directories[result_window[1] + k] .. '\r\n'
                    end
                end
            end
            result = result .. '<<<\r\n'
        else
            result = 'ok >>>\r\n' .. table.concat(path, '\r\n') .. '\r\n' .. '<<<\r\n'
        end
    else
        return nil, 'No root paths found'
    end
    return result
end

return _M
