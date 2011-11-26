--[[
Copyright 2011 by Anton Kochkov <anton.kochkov@gmail.com>

This file is part of Bvim.

Bvim is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Bvim is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bvim.  If not, see <http://www.gnu.org/licenses/>.
--]]

local constants_store = {
  VISUAL_SELECTION_ID = BVI_VISUAL_SELECTION_ID,
  MODE_CMD = BVI_MODE_CMD,
  MODE_EDIT = BVI_MODE_EDIT,
  MODE_VISUAL = BVI_MODE_VISUAL,
  MODE_REPL = BVI_MODE_REPL,
}

local const = newproxy(true)

getmetatable(const).__index = function(t,k) return constants_store[k] end
getmetatable(const).__newindex = function() error 'attempted to overwrite a constant!' end
getmetatable(const).__metatable = false

-- Using constants as const.MODE_CMD , for example
