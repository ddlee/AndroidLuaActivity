module(..., package.seeall);

require('Wall');
require('Painter');

WIDTH = 32;

local mt = getfenv();
mt.__index = mt;

function new()
   local obj = {};

   obj.x = Wall.WIDTH/2;

   setmetatable(obj, mt);
   return obj;
end

function draw(self, p)
   p:setColor(Painter.ORANGE);
   p:bar(self.x - WIDTH/2, Wall.HEIGHT - 2,
	 self.x + WIDTH/2, Wall.HEIGHT);
end

function tick(self, ball)
   local f = {x=0, y=0};
   local s = ball.x - self.x + WIDTH/2;
   if ((s >= 0) and (s < WIDTH)
     and (Wall.HEIGHT - 3 - ball.y < 0)) then
      f.x = (Wall.HEIGHT-3-ball.y)*(self.x-ball.x)/WIDTH;
      f.y = Wall.HEIGHT-3-ball.y;
   end

   return f;
end

function setX(self, x)
   self.x = x;
end
