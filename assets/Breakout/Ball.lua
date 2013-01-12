module(..., package.seeall);

require('Wall');

DT = 0.001;

local mt = getfenv();
mt.__index = mt;

function new()
   local obj = {};

   obj.x = Wall.WIDTH/2;
   obj.y = Wall.HEIGHT - 3;
   obj.vx = 80;
   obj.vy = -80;

   setmetatable(obj, mt);
   return obj;
end

function draw(self, p)
   p:setColor(Painter.WHITE);
   p:ball(self.x, self.y);
end

function tick(self, f)
   local x = f.x;
   local y = f.y;
   local LIM = .2;
   if (x > LIM) then x = LIM; end
   if (x < -LIM) then x = -LIM; end
   if (y > LIM) then y = LIM; end
   if (y < -LIM) then y = -LIM; end

   self.vx = self.vx + 20*x;
   self.vy = self.vy + 20*y;
   self.x = self.x + DT*self.vx;
   self.y = self.y + DT*self.vy;
end


