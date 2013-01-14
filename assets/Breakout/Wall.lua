module(..., package.seeall);

require('Brick');

EXPAND = 1;
ROWS_COUNT = 16 * EXPAND;
COLS_COUNT = 8*3 * EXPAND;

WIDTH = ROWS_COUNT * Brick.WIDTH;
HEIGHT = COLS_COUNT * Brick.HEIGHT;

local mt = getfenv();
mt.__index = mt;

function new()
   local obj = {};

   obj.bricks = {};
   local row, col;
   for row = 0,ROWS_COUNT/3 do
      for col = 0,COLS_COUNT-1 do
	 obj.bricks[#obj.bricks + 1] = Brick.new(col, row);
      end
   end

   setmetatable(obj, mt);
   return obj;
end

function draw(self, p)
   local i;
   for i = 1,#self.bricks do
      self.bricks[i]:draw(p);
   end
end

function tick(self, ball)
   f = {x=0, y=0};
   if (ball.x < 1) then
      f.x = f.x + (1 - ball.x);
   end
   if (ball.x > WIDTH - 1) then
      f.x = f.x + (WIDTH - 1 - ball.x);
   end
   if (ball.y < 1) then
      f.y = f.y + (1 - ball.y);
   end

   local i;
   for i = 1,#self.bricks do
      local fbrick = self.bricks[i]:tick(ball);
      f.x = f.x + fbrick.x;
      f.y = f.y + fbrick.y;
      if ((fbrick.x ~= 0) or (fbrick.y ~= 0)) then
	 self.bricks[i]:destroy();
      end
   end

   return f;
end
