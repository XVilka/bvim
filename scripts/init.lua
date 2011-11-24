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
