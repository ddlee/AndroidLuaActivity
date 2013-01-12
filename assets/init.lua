print("Starting init.lua")
require("asset")

--Modify global dofile() function
_dofile = _G.dofile;
dofile = asset.dofile;
--[[
function dofile(filename)
   local lf, err;
   lf, err = loadfile(filename);
   if (lf) then
      return lf();
   else
      lf, err = asset.loadfile(filename);
      if (lf) then
	 return lf()
      else
	 print(err);
      end
   end
end
--]]

--Start up main.lua:
-- Setup path for require() loader
asset.path = "Breakout/?.lua;"..asset.path;

dofile("Breakout/breakout.lua");
