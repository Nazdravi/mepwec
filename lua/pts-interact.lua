-- (C) Arndt Kritzner, 2021

local _M = {}

-- Beispiel: https://luaposix.github.io/luaposix/examples/termios.lua.html

function _M.exec(dev, command)
    local M = require 'posix.termio'
    local F = require 'posix.fcntl'
    local U = require 'posix.unistd'
    local T = require 'posix.time';
    
    local result = ''
    
    -- open terminal
    local fds, err = F.open(dev, F.O_RDWR + F.O_NONBLOCK)
    if fds ~= nil then
        -- disable local echo
        local termios, err = M.tcgetattr(fds)
        if termios == nil then
            return nil, 'Unable to get terminal attributes: ' .. err
        end
        local lflags = { termios.lflag % (M.ECHO * 2), termios.lflag % M.ECHO, } -- clip right-side bits with and without ECHO bit
        termios.lflag = termios.lflag - lflags[1] + lflags[2]
        local r, err = M.tcsetattr(fds, 0, termios)
        if r == nil then
            return nil, 'Unable to set terminal attributes: ' .. err
        end
        if string.len(command) > 0 then
            local written = false
            local received = false
            local start = T.clock_gettime(T.CLOCK_MONOTONIC);
            while true do
                received = false;
                local fd_list = {
                    [fds] = {events={IN =true}},
                }
                local ready, err = require 'posix.poll'.poll(fd_list,100)
                if ready == 1 then
                    -- read response
                    local data, err = U.read(fds, 1024)
                    if not data then
                        return nil, 'Error while reading from terminal: ' .. err
                    end
                    if string.len(data) > 0 then
                        received = true
                    end
                    if written and received then
                        result = result .. string.gsub(data, '\r', '')
                        if string.find(result, '[^\n]+\n') then
                            if string.find(result, ' >>>\n') then
                                if string.find(result, '\n<<<\n') then
                                    break;
                                end
                            else
                                break;
                            end
                        end
                    end
                end
                if not (received or written) then
                    -- send command
                    local ok, err = U.write(fds, command .. '\r\n')
                    if not ok then
                        return nil, 'Unable to write command to terminal: ' .. err
                    end
                    written = true
                end
                local now = T.clock_gettime(T.CLOCK_MONOTONIC);
                if (now.tv_sec > start.tv_sec + 3) or ((now.tv_sec == start.tv_sec + 3) and (now.tv_nsec > start.tv_nsec)) then
                    break
                end
            end
        end
        U.close(fds)
    else
        return nil, 'Could not open terminal ' .. dev .. ':' .. err
    end
    
    result = string.gsub(result, '[\r\n]+', '\n')
    return result
end

return _M
