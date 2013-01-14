module(..., package.seeall);

WIDTH = 20;
HEIGHT = 8;

local mt = getfenv();
mt.__index = mt;

function new(col, row)
   local obj = {};

   obj.col = col;
   obj.row = row;
   obj.countDownTimer = -1;

   setmetatable(obj, mt);
   return obj;
end

function draw(self, p)
   local iColor;
   if (self.countDownTimer == -1) then
      iColor = (math.floor(self.row/2) % Painter.nColor) + 1;
      p:setColor(iColor);
      p:bar(self.col*WIDTH+1, self.row*HEIGHT+1,
	    (self.col+1)*WIDTH-1, (self.row+1)*HEIGHT-1);
   elseif (self.countDownTimer ~= 0) then
      iColor = (math.floor(self.countDownTimer/10) % Painter.nColor) + 1;
      p.setColor(iColor);
      p:bar(self.col*WIDTH+1, self.row*HEIGHT+1,
	    (self.col+1)*WIDTH-1, (self.row+1)*HEIGHT-1);
   end
end

function tick(self, ball)
   local f = {x=0, y=0};
   if (self.countDownTimer == 0) then
      return f;
   end
   if (self.countDownTimer > 0) then
      self.countDownTimer = self.countDownTimer - 1;
   end

   local f1 = (ball.y - self.row * HEIGHT) * WIDTH -
     (ball.x - self.col * WIDTH) * HEIGHT;
   local f2 = (ball.y - self.row * HEIGHT - HEIGHT) * WIDTH +
     (ball.x - self.col * WIDTH) * HEIGHT;
   local d = (f1 < 0);
   local u = (f2 < 0);
   if (d and u) then -- top
      if (self.row * HEIGHT - ball.y - 1 < 0) then
	 f.y = f.y + self.row * HEIGHT - ball.y - 1;
      end
   elseif (d and not u) then -- right
      if (self.col * WIDTH + WIDTH - ball.x + 1 > 0) then
	 f.x = f.x + self.col * WIDTH + WIDTH - ball.x + 1;
      end
   elseif (not d and u) then -- left
      if (self.col *WIDTH - ball.x - 1 < 0) then
	 f.x = f.x + self.col * WIDTH - ball.x - 1;
      end
   else -- bottom
      if (self.row *HEIGHT + HEIGHT - ball.y + 1 > 0) then
	 f.y = f.y + self.row * HEIGHT + HEIGHT - ball.y + 1;
      end
   end

   return f;
end

function destroy(self)
   if (self.countDownTimer == -1) then
      self.countDownTimer = 60;
   end
end
