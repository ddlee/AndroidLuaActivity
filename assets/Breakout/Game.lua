module(..., package.seeall);

require('Wall');
require('Ball');
require('Paddle');

local mt = getfenv();
mt.__index = mt;

function new()
   local obj = {};

   obj.wall = Wall.new();
   obj.ball = Ball.new();
   obj.paddle = Paddle.new();

   setmetatable(obj, mt);
   return obj;
end

function draw(self, p)
   self.wall:draw(p);
   self.ball:draw(p);
   self.paddle:draw(p);
end

function setX(self, x)
   self.paddle:setX(x);
end

function tick(self)
   f = self.wall:tick(self.ball);
   fpaddle = self.paddle:tick(self.ball);
   f.x = f.x + fpaddle.x;
   f.y = f.y + fpaddle.y;

   self.ball:tick(f);
   if (self.ball.y > Wall.HEIGHT) then
      self.ball = Ball.new();
   end
end
