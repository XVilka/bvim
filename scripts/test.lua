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

bvim.exec("set color(data,999,000,000)")
bvim.display_error("here is an example error message!")
bvim.msg_window("please, press any key to exit window")
bvim.scrolldown(5)
bvim.print(8, 15, 4, "here is lua scripting test")
bvim.exec("block add 1 12 233 4")
bvim.exec("block add 2 357 683 2")
bvim.exec("block add 3 749 919 3")
bvim.block_xor(1, 15)
bvim.block_and(2, 14)
bvim.block_or(3, 255)
bvim.block_lshift(2, 2)
bvim.block_and(3, 26)
bvim.block_rrotate(3, 3)
bvim.exec("run checksum")
shasums(15, 200)

