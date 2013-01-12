print("Starting main.lua")

require('glut');
require('gles1');
require('toast');
require('unix');

function onStart()
   toast.new("onStart"):show();
end

function display()
   print("display")
   gl.ClearColor(0, 0.2, 0.8, 1);
   gl.Clear(gl.COLOR_BUFFER_BIT);
 
   glut.SwapBuffers();
end

function reshape(w,h)
   print("reshape",w,h)
   gl.Viewport(0, 0, w, h);
end

function timer(arg)
   print("timer", unix.time());
   glut.TimerFunc(1000, timer, 0);
end

function mouse(x,y)
   print("mouse:",x,y);
end

glut.Init()
glut.InitDisplayMode(glut.DOUBLE + glut.RGB);
glut.InitWindowSize(400,400);
glut.CreateWindow('Breakout')

glut.DisplayFunc(display);
glut.ReshapeFunc(reshape);
glut.PassiveMotionFunc(mouse);

timer();
glut.MainLoop();


