require('glut');
require('gles1');

require('Painter');
require('Game');
require('Wall');
require('Ball');

game = Game.new();
p = Painter.new();

width = Wall.WIDTH;
height = Wall.HEIGHT;

winCreated = false;

function display()
   gl.Clear(gl.COLOR_BUFFER_BIT);

   game:draw(p);

   glut.SwapBuffers();
end

function reshape(w, h)
   width = w;
   height = h;
   winCreated = true;

--   gl.Viewport(0, 0, w, h);
   gl.Viewport(0, 0, w, h-20);

   gl.ClearColor(0, 0, 0, 1.0);
   gl.MatrixMode(gl.PROJECTION);
   gl.LoadIdentity();
   gl.Orthof(0, Wall.WIDTH, Wall.HEIGHT, 0, -1.0, 1.0);
   gl.MatrixMode(gl.MODELVIEW);
   gl.LoadIdentity();
end


function timer(t)
   for i = 0, 1/25/Ball.DT do
      game:tick();
   end
   if (winCreated) then
      display();
   end
   glut.TimerFunc(1000/25, timer, 0);
end

function mouse(x,y)
   game:setX((Wall.WIDTH/width)*x);
end

glut.Init();
glut.InitDisplayMode(glut.DOUBLE + glut.RGB);
glut.InitWindowSize(Wall.WIDTH, Wall.HEIGHT);
glut.CreateWindow('Breakout');
glut.DisplayFunc(display);
glut.ReshapeFunc(reshape);
glut.PassiveMotionFunc(mouse);
glut.WMCloseFunc(function() os.exit() end);
timer();

glut.MainLoop();
