/*
 * This file was automatically generated, do not edit.
 */

#include <geogram_gfx/glup_viewer/glup_viewer_lua.h>

void register_embedded_lua_files(void);
   
void register_embedded_lua_files() {
     register_embedded_lua_file("lib/preamble.lua",
        "--             preamble.lua \n"
        "-- FR bibliotheque interne, incluse par defaut \n"
        "-- FR ce fichier est toujours execute avant votre programme \n"
        "-- EN internal library, included by default \n"
        "-- EN this file is always executed before your program \n"
        " \n"
        "-- FR une fonction qui ne fait rien \n"
        "-- EN a function that does nothing \n"
        "function NOP() \n"
        "end \n"
        " \n"
        "-- EN by-default definition for functions called by GEOCOD \n"
        "-- FR definition par defaut des fonctions appelees par GEOCOD \n"
        "GLUP.draw_scene = NOP \n"
        "imgui.draw_application_menus = NOP \n"
        "imgui.draw_object_properties = NOP \n"
        "imgui.on_key_pressed = NOP \n"
        "imgui.on_key_released = NOP \n"
        " \n"
        " \n"
        "function GLUP.init_graphics() \n"
        "    GLUP.SetRegionOfInterest(0,0,0,1,1,1) \n"
        "end \n"
        " \n"
        " \n"
        "GLUP.ResetViewer() \n"
        " \n"
        " \n"
     );

     register_embedded_lua_file("lib/pixel.lua",
        "--             pixel.lua \n"
        "-- FR bibliotheque interne, incluse quand on appelle import(\"pixel\") \n"
        "-- FR fournit des fonctions simples pour afficher des (gros) pixels \n"
        "-- EN internal library, included when one calls import(\"pixel\") \n"
        "-- EN defines easy-to-use functions for displaying (big) pixels \n"
        " \n"
        "function GLUP.Pixel3D(x,y,z) \n"
        "   GLUP.Vertex(x,y,z) \n"
        "   GLUP.Vertex(x+1,y,z) \n"
        "   GLUP.Vertex(x,y+1,z) \n"
        "   GLUP.Vertex(x+1,y+1,z) \n"
        "   GLUP.Vertex(x,y,z+1) \n"
        "   GLUP.Vertex(x+1,y,z+1) \n"
        "   GLUP.Vertex(x,y+1,z+1) \n"
        "   GLUP.Vertex(x+1,y+1,z+1) \n"
        "end \n"
        " \n"
        "function GLUP.Pixel2D(x,y) \n"
        "   GLUP.Pixel3D(x,y,0.0) \n"
        "end \n"
        " \n"
        "pix = GLUP.Pixel2D \n"
        "pix3d = GLUP.Pixel3D \n"
        "col = GLUP.Color \n"
        " \n"
        "function pixBegin() \n"
        "   GLUP.Enable(GLUP.VERTEX_COLORS) \n"
        "   GLUP.Begin(GLUP.HEXAHEDRA) \n"
        "end \n"
        " \n"
        "function pixEnd() \n"
        "   GLUP.End() \n"
        "   GLUP.Disable(GLUP.VERTEX_COLORS) \n"
        "end \n"
        " \n"
        "function pixGrid() \n"
        "   GLUP.SetColor(GLUP.FRONT_COLOR,0,0,0.5) \n"
        "   GLUP.Begin(GLUP.LINES) \n"
        "   local xm,ym,zm,xM,yM,zM \n"
        "   xm,ym,zm,xM,yM,zM = GLUP.GetRegionOfInterest() \n"
        "   for x=xm,xM,1 do \n"
        "      GLUP.Vertex(x,ym,0) \n"
        "      GLUP.Vertex(x,yM,0) \n"
        "   end \n"
        "   for y=ym,yM,1 do \n"
        "      GLUP.Vertex(xm,y,0) \n"
        "      GLUP.Vertex(xM,y,0) \n"
        "   end \n"
        "   GLUP.End() \n"
        "end \n"
        " \n"
        "function GLUP.init_graphics() \n"
        "   GLUP.SetRegionOfInterest(1,1,1,21,21,1) \n"
        "end \n"
     );

     register_embedded_lua_file("lib/turtle.lua",
        "--             turtle.lua \n"
        "-- FR bibliotheque interne, incluse quand on appelle import(\"turtle\") \n"
        "-- FR fournit des fonctions pour l'affichage graphique en mode \"tortue\" \n"
        "-- EN internal library, included when one calls import(\"turtle\") \n"
        "-- EN defines functions for \"turtle\" graphics \n"
        " \n"
        "turtle={} \n"
        " \n"
        "function turtle.pendown() \n"
        "   if not turtle.pen then \n"
        "      turtle.pen=true \n"
        "      turtle.first=true \n"
        "      GLUP.Enable(GLUP.VERTEX_COLORS) \n"
        "      GLUP.Begin(GLUP.QUADS) \n"
        "   end \n"
        "end \n"
        " \n"
        "function turtle.penup() \n"
        "   if turtle.pen then \n"
        "      turtle.pen=false \n"
        "      GLUP.End() \n"
        "      GLUP.Disable(GLUP.VERTEX_COLORS) \n"
        "   end \n"
        "end \n"
        " \n"
        "function turtle.forward(dist) \n"
        "   local new_x = turtle.x + math.cos(turtle.alpha)*dist \n"
        "   local new_y = turtle.y + math.sin(turtle.alpha)*dist \n"
        "   if turtle.pen then \n"
        "      local Nx = turtle.width * (turtle.y - new_y) / dist \n"
        "      local Ny = turtle.width * (new_x - turtle.x) / dist \n"
        "      GLUP.Vertex(turtle.x+Nx,turtle.y+Ny,0) \n"
        "      GLUP.Vertex(turtle.x-Nx,turtle.y-Ny,0) \n"
        "      GLUP.Vertex(new_x-Nx, new_y-Ny,0) \n"
        "      GLUP.Vertex(new_x+Nx, new_y+Ny,0) \n"
        "   end \n"
        "   turtle.x = new_x \n"
        "   turtle.y = new_y \n"
        "end \n"
        " \n"
        "function turtle.backward(dist) \n"
        "   turtle.alpha = turtle.alpha + math.pi \n"
        "   turtle.forward(dist) \n"
        "   turtle.alpha = turtle.alpha - math.pi \n"
        "end \n"
        " \n"
        "function turtle.left(dalpha) \n"
        "   turtle.alpha = turtle.alpha + dalpha * math.pi / 180 \n"
        "end \n"
        " \n"
        "function turtle.right(dalpha) \n"
        "   turtle.alpha = turtle.alpha - dalpha * math.pi / 180 \n"
        "end \n"
        " \n"
        "function turtle.home() \n"
        "   turtle.x=0 \n"
        "   turtle.y=0 \n"
        "   turtle.alpha=math.pi/2 \n"
        "   if turtle.pen then \n"
        "      turtle.penup() \n"
        "   end \n"
        "   turtle.width=0.2 \n"
        "   turtle.first=true \n"
        "end \n"
        " \n"
        "function turtle.penwidth(w) \n"
        "   turtle.width = w \n"
        "end \n"
        " \n"
        "turtle.pencolor = GLUP.Color \n"
        " \n"
        "-- FR fonctions \"raccourcis\" \n"
        "-- FR (ca va plus vite a taper !) \n"
        "-- EN shorthands \n"
        " \n"
        "pu = turtle.penup \n"
        "pd = turtle.pendown \n"
        "fd = turtle.forward \n"
        "bk = turtle.backward \n"
        "tl = turtle.left \n"
        "tr = turtle.right \n"
        "home = turtle.home \n"
        "pcol = turtle.pencolor \n"
        "pwidth = turtle.penwidth \n"
        " \n"
        "turtle.pu = turtle.penup \n"
        "turtle.pd = turtle.pendown \n"
        "turtle.fd = turtle.forward \n"
        "turtle.bk = turtle.backward \n"
        "turtle.tl = turtle.left \n"
        "turtle.tr = turtle.right \n"
        "turtle.pcol = turtle.pencolor \n"
        "turtle.pwidth = turtle.penwidth \n"
        " \n"
        "function GLUP.init_graphics() \n"
        "   GLUP.SetRegionOfInterest(-50,-50,1,50,50,1) \n"
        "end \n"
        " \n"
     );

     register_embedded_lua_file("templates/pixel_program.lua",
        "import(\"pixel\") \n"
        " \n"
        "function GLUP.init_graphics() \n"
        "   GLUP.SetRegionOfInterest(1,1,1,11,11,1) \n"
        "end \n"
        " \n"
        "function GLUP.draw_scene() \n"
        " \n"
        "    GLUP.Enable(GLUP.DRAW_MESH) \n"
        "    GLUP.SetCellsShrink(0.1) \n"
        "    pixGrid() \n"
        " \n"
        "    pixBegin() \n"
        "    col(\"blue\") \n"
        "    pix(1,1) \n"
        "    pixEnd() \n"
        "end \n"     );

     register_embedded_lua_file("templates/turtle_program.lua",
        "import(\"turtle\") \n"
        " \n"
        "function GLUP.draw_scene() \n"
        "    home() \n"
        "    pcol(\"black\") \n"
        "    pd() \n"
        "    fd(100) \n"
        "    pu() \n"
        "end \n"     );

     register_embedded_lua_file("examples/arbre.lua",
        "import(\"turtle\") \n"
        " \n"
        "a=30 \n"
        "level=5 \n"
        "r=0 \n"
        "lr=0.3 \n"
        "threed=false \n"
        " \n"
        "function feuille() \n"
        "    local h=30/math.exp(level*lr) \n"
        "    pwidth(h/3) \n"
        "    pcol(0,1,0) \n"
        "    local a = math.random(-r,r) \n"
        "    tr(a) \n"
        "    fd(h) \n"
        "    tr(180) \n"
        "    pwidth(0.001) \n"
        "    pcol(0,0,0) \n"
        "    fd(h) \n"
        "    tl(180) \n"
        "    tl(a) \n"
        "end \n"
        " \n"
        "function arbre(i) \n"
        "   local b = a + math.random(-r,r) \n"
        "   local h = 30*math.exp(i*lr)/math.exp(level*lr) \n"
        "   pwidth(h*0.05) \n"
        "   pcol(0,i/level,1-i/level) \n"
        "   fd(h) \n"
        "   if i>1 then \n"
        "      tl(b) \n"
        "      arbre(i-1) \n"
        "      tr(b) \n"
        "      tr(b) \n"
        "      arbre(i-1) \n"
        "      tl(b) \n"
        "   else \n"
        "      feuille() \n"
        "   end \n"
        "   tr(180) \n"
        "   fd(h) \n"
        "   tl(180) \n"
        "end \n"
        " \n"
        "function GLUP.draw_scene() \n"
        "   if threed then \n"
        "      GLUP.Enable(GLUP.LIGHTING) \n"
        "   else \n"
        "      GLUP.Disable(GLUP.LIGHTING) \n"
        "   end \n"
        "   math.randomseed(0) \n"
        "   home() \n"
        "   pcol(\"green\") \n"
        " \n"
        "   if threed then \n"
        "       GLUP.MatrixMode(GLUP.MODELVIEW_MATRIX) \n"
        "       GLUP.PushMatrix() \n"
        "       GLUP.Rotate(os.clock()*50,0,1,0) \n"
        "   end \n"
        " \n"
        "   tr(180) \n"
        "   fd(20) \n"
        "   tr(180) \n"
        "   bk(30) \n"
        " \n"
        "   pd() \n"
        "   arbre(level) \n"
        "   pu() \n"
        " \n"
        "   if threed then \n"
        "       for i=1,3,1 do \n"
        "	   GLUP.Rotate(120,0,1,0) \n"
        "	   pd() \n"
        "	   arbre(level) \n"
        "	   pu() \n"
        "       end \n"
        "       GLUP.PopMatrix() \n"
        "    end \n"
        "end \n"
        " \n"
        "function imgui.draw_object_properties() \n"
        "   local b \n"
        "   b,a = imgui.SliderFloat(\"angle\", a, 0, 180, \"%3f\", 1.0) \n"
        "   b,r = imgui.SliderInt(\"rnd\", r, 0, 100, \"%.0f\") \n"
        "   b,level = imgui.SliderInt(\"level\", level, 1, 15, \"%.0f\") \n"
        "   b,lr = imgui.SliderFloat(\"ratio\", lr, 0.1, 0.5, \"%3f\", 1.0) \n"
        "   b,threed = imgui.Checkbox(\"3D\",threed) \n"
        "end \n"
     );

     register_embedded_lua_file("examples/flake.lua",
        "import(\"turtle\") \n"
        " \n"
        "function VonKoch(level) \n"
        "   if level==1 then \n"
        "      fd(1) \n"
        "   else \n"
        "      VonKoch(level-1) \n"
        "      tl(60) \n"
        "      VonKoch(level-1) \n"
        "      tr(120) \n"
        "      VonKoch(level-1) \n"
        "      tl(60) \n"
        "      VonKoch(level-1) \n"
        "   end \n"
        "end \n"
        " \n"
        "function Flake(level) \n"
        "   pd() \n"
        "   VonKoch(level) \n"
        "   tr(120) \n"
        "   VonKoch(level) \n"
        "   tr(120) \n"
        "   VonKoch(level) \n"
        "   pu() \n"
        "end \n"
        " \n"
        "function GLUP.draw_scene() \n"
        "    home() \n"
        "    fd(40) \n"
        "    tl(90) \n"
        "    fd(40) \n"
        "    tr(90) \n"
        "    pwidth(0.1) \n"
        "    tr(90) \n"
        "    GLUP.Enable(GLUP.VERTEX_COLORS) \n"
        "    GLUP.Disable(GLUP.LIGHTING) \n"
        "    pcol(\"white\") \n"
        "    Flake(5) \n"
        "end \n"
     );

     register_embedded_lua_file("examples/sierpinski.lua",
        "import(\"turtle\") \n"
        " \n"
        " \n"
        "function Sierpinski(a, level) \n"
        "   if level==0 then \n"
        "    pd() \n"
        "	for i=1,3,1 do \n"
        "      	 fd(a) \n"
        "      	 tl(120) \n"
        "	end \n"
        "   pu() \n"
        "   else \n"
        "	Sierpinski(a/2, level - 1) \n"
        "	pu() \n"
        "	fd(a/2) \n"
        "	pd() \n"
        "	Sierpinski(a/2, level - 1) \n"
        "	pu() \n"
        "	tl(120) \n"
        "	fd(a/2) \n"
        "	tr(120) \n"
        "	pd() \n"
        "	Sierpinski(a/2, level - 1) \n"
        "    	pu() \n"
        "	-- We should return home! \n"
        "	pd() \n"
        "	tl(60) \n"
        "	bk(a/2) \n"
        "	tr(60) \n"
        "	pu() \n"
        "   end \n"
        "end \n"
        " \n"
        " \n"
        "function GLUP.draw_scene() \n"
        "    home() \n"
        "    pwidth(0.2) \n"
        "    GLUP.Enable(GLUP.VERTEX_COLORS) \n"
        "    GLUP.Disable(GLUP.LIGHTING) \n"
        "    pcol(\"white\") \n"
        "    Sierpinski(100,5) \n"
        "end \n"
     );

     register_embedded_lua_file("examples/sponge.lua",
        "import(\"pixel\") \n"
        " \n"
        "N=3*3 \n"
        " \n"
        "function sponge(x,y,z,d) \n"
        "  if d < 1 then \n"
        "     pix3d(x,y,z) \n"
        "  else \n"
        "     sponge(x,y,z,d/3) \n"
        "     sponge(x+d,y,z,d/3) \n"
        "     sponge(x+2*d,y,z,d/3) \n"
        "     sponge(x,y+d,z,d/3) \n"
        "     sponge(x+2*d,y+d,z,d/3) \n"
        "     sponge(x,y+2*d,z,d/3) \n"
        "     sponge(x+d,y+2*d,z,d/3) \n"
        "     sponge(x+2*d,y+2*d,z,d/3) \n"
        "     sponge(x,y,z+d,d/3) \n"
        "     sponge(x+2*d,y,z+d,d/3) \n"
        "     sponge(x,y+2*d,z+d,d/3) \n"
        "     sponge(x+2*d,y+2*d,z+d,d/3) \n"
        "     sponge(x,y,z+2*d,d/3) \n"
        "     sponge(x+d,y,z+2*d,d/3) \n"
        "     sponge(x+2*d,y,z+2*d,d/3) \n"
        "     sponge(x,y+d,z+2*d,d/3) \n"
        "     sponge(x+2*d,y+d,z+2*d,d/3) \n"
        "     sponge(x,y+2*d,z+2*d,d/3) \n"
        "     sponge(x+d,y+2*d,z+2*d,d/3) \n"
        "     sponge(x+2*d,y+2*d,z+2*d,d/3) \n"
        "  end \n"
        "end \n"
        " \n"
        "function GLUP.init_graphics() \n"
        "  GLUP.SetRegionOfInterest(1,1,1,N*3,N*3,N*3) \n"
        "end \n"
        " \n"
        "function GLUP.draw_scene() \n"
        "  GLUP.Enable(GLUP.DRAW_MESH) \n"
        "  pixBegin() \n"
        "  col(\"yellow\") \n"
        "  sponge(1,1,1,N) \n"
        "  pixEnd() \n"
        "end \n"
     );

     register_embedded_lua_file("examples/creeper.lua",
        "-- Dessin d'un \"creeper\" \n"
        "-- Par Nathan Levy, Decembre 2016 \n"
        " \n"
        "import(\"pixel\") \n"
        " \n"
        "function GLUP.draw_scene() \n"
        "   GLUP.SetCellsShrink(0.1) \n"
        "   GLUP.Enable(GLUP.DRAW_MESH) \n"
        "   GLUP.Enable(GLUP.VERTEX_COLORS) \n"
        "   pixBegin(); \n"
        "   col(\"green\") \n"
        "   pix(1,1) \n"
        "   pix(2,1) \n"
        "   pix(3,1) \n"
        "   pix(4,1) \n"
        "   pix(5,1) \n"
        "   pix(6,1) \n"
        "   pix(7,1) \n"
        "   pix(8,1) \n"
        "   pix(1,2) \n"
        "   col(\"black\") \n"
        "   pix(2,2) \n"
        "   pix(3,2) \n"
        "   col(\"green\") \n"
        "   pix(4,2) \n"
        "   pix(5,2) \n"
        "   col(\"black\") \n"
        "   pix(6,2) \n"
        "   pix(7,2) \n"
        "   col(\"green\") \n"
        "   pix(8,2) \n"
        "   pix(1,3) \n"
        "   col(\"black\") \n"
        "   for x=1,7,1 do \n"
        "      pix(x,3) \n"
        "   end \n"
        "   col(\"green\") \n"
        "   pix(8,3) \n"
        "   pix(1,4) \n"
        "   pix(2,4) \n"
        "   pix(3,4) \n"
        "   col(\"black\") \n"
        "   pix(4,4) \n"
        "   pix(5,4) \n"
        "   col(\"green\") \n"
        "   pix(6,4) \n"
        "   pix(7,4) \n"
        "   pix(8,4) \n"
        "   for x=1,8,1 do \n"
        "      pix(x,5) \n"
        "   end \n"
        "   pix(1,6) \n"
        "   col(\"black\") \n"
        "   pix(2,6) \n"
        "   pix(3,6) \n"
        "   col(\"green\") \n"
        "   pix(4,6) \n"
        "   pix(5,6) \n"
        "   col(\"black\") \n"
        "   pix(6,6) \n"
        "   pix(7,6) \n"
        "   col(\"green\") \n"
        "   pix(8,6) \n"
        "   pix(1,7) \n"
        "   col(\"black\") \n"
        "   pix(2,7) \n"
        "   pix(3,7) \n"
        "   col(\"green\") \n"
        "   pix(4,7) \n"
        "   pix(5,7) \n"
        "   col(\"black\") \n"
        "   pix(6,7) \n"
        "   pix(7,7) \n"
        "   col(\"green\") \n"
        "   pix(8,7) \n"
        "   for x=1,8,1 do \n"
        "      pix(x,8) \n"
        "   end \n"
        " \n"
        "   pixEnd() \n"
        "end \n"
     );

     register_embedded_lua_file("games/hackman.lua",
        "import(\"pixel\") \n"
        " \n"
        "GLUP.ArcadeStyle() \n"
        " \n"
        "joueurx=13 \n"
        "joueury=10 \n"
        "vx=0 \n"
        "vy=0 \n"
        "newvx=0 \n"
        "newvy=0 \n"
        "lasttime=0 \n"
        "lastfantomtime=0 \n"
        " \n"
        "gumtime=0 \n"
        "gummode=false \n"
        "eatentime=0 \n"
        "eatenmode=false \n"
        "frozen=true \n"
        "gameover=false \n"
        " \n"
        "fantomx=13 \n"
        "fantomy=15 \n"
        "fantomvx=0 \n"
        "fantomvy=1 \n"
        " \n"
        " \n"
        "score=0 \n"
        "level=1 \n"
        "lives=3 \n"
        " \n"
        "niveau = { \n"
        "\"*************************\", \n"
        "\"*...........*...........*\", \n"
        "\"*.****.****.*.****.****.*\", \n"
        "\"*O*  *.*  *.*.*  *.*  *O*\", \n"
        "\"*.****.****.*.****.****.*\", \n"
        "\"*.......................*\", \n"
        "\"*.****.*.*******.*.****.*\", \n"
        "\"*......*....*....*......*\", \n"
        "\"******.**** * ****.******\", \n"
        "\"     *.*         *.*     \", \n"
        "\"******.* ******* *.******\", \n"
        "\"      .  *     *  .      \", \n"
        "\"******.* *     * *.******\", \n"
        "\"     *.* ******* *.*     \", \n"
        "\"     *.*         *.*     \", \n"
        "\"******.* ******* *.******\", \n"
        "\"*...........*...........*\", \n"
        "\"*.****.****.*.****.****.*\", \n"
        "\"*O...*.............*...O*\", \n"
        "\"****.*.*.*******.*.*.****\", \n"
        "\"*......*....*....*......*\", \n"
        "\"*.*********.*.*********.*\", \n"
        "\"*.......................*\", \n"
        "\"*************************\" \n"
        "} \n"
        " \n"
        "niveauH=#niveau \n"
        "niveauL=#niveau[1] \n"
        " \n"
        "function youwin() \n"
        "    niveau = { \n"
        "    \"O.O.O.O.O.O.O.O.O.O.O.O.O\", \n"
        "    \".                       .\", \n"
        "    \"O  *   *    ***   *  *  O\", \n"
        "    \".  *   *   *   *  *  *  .\", \n"
        "    \"O  *   *   *   *  *  *  O\", \n"
        "    \".   * *    *   *  *  *  .\", \n"
        "    \"O    *     *   *  *  *  O\", \n"
        "    \".    *     *   *  *  *  .\", \n"
        "    \"O    *     *   *  *  *  O\", \n"
        "    \".    *      ***    **   .\", \n"
        "    \"O                       O\", \n"
        "    \".                       .\", \n"
        "    \"O                       O\", \n"
        "    \".  * * *  ***  **    *  .\", \n"
        "    \"O  * * *   *   * *   *  O\", \n"
        "    \".  * * *   *   * *   *  .\", \n"
        "    \"O  * * *   *   *  *  *  O\", \n"
        "    \".  * * *   *   *  *  *  .\", \n"
        "    \"O  * * *   *   *   * *  O\", \n"
        "    \".  * * *   *   *   * *  .\", \n"
        "    \"O   ***   ***  *    **  O\", \n"
        "    \".                       .\", \n"
        "    \"O.O.O.O.O.O.O.O.O.O.O.O.O\" \n"
        "    } \n"
        "    niveauH=#niveau \n"
        "    niveauL=#niveau[1] \n"
        "    gameover=true \n"
        "end \n"
        " \n"
        "function youlose() \n"
        "    niveau = { \n"
        "    \"O.O.O.O.O.O.O.O.O.O.O.O.O\", \n"
        "    \".                       .\", \n"
        "    \"O       *********       O\", \n"
        "    \".      * ... ... *      .\", \n"
        "    \"O     *   O   O   *     O\", \n"
        "    \".    *             *    .\", \n"
        "    \"O    *  .       .  *    O\", \n"
        "    \".    *   .......   *    .\", \n"
        "    \"O    *             *    O\", \n"
        "    \".    * **  **  **  *    .\", \n"
        "    \"O    **  **  **  ***    O\", \n"
        "    \".                       .\", \n"
        "    \"O                       O\", \n"
        "    \".  ****    ***    ***   .\", \n"
        "    \"O  *   *  *   *  *   *  O\", \n"
        "    \".  *   *  *   *  *   *  .\", \n"
        "    \"O  ****   *   *  *   *  O\", \n"
        "    \".  *   *  *   *  *   *  .\", \n"
        "    \"O  *   *  *   *  *   *  O\", \n"
        "    \".  *   *  *   *  *   *  .\", \n"
        "    \"O  ****    ***    ***   O\", \n"
        "    \".                       .\", \n"
        "    \"O.O.O.O.O.O.O.O.O.O.O.O.O\" \n"
        "    } \n"
        "    niveauH=#niveau \n"
        "    niveauL=#niveau[1] \n"
        "    gameover=true \n"
        "end \n"
        " \n"
        " \n"
        "function getniveau(x,y) \n"
        "   return niveau[#niveau-y+1]:sub(x,x) \n"
        "end \n"
        " \n"
        "function setniveau(x,y,c) \n"
        "    local s = niveau[#niveau-y+1] \n"
        "    s = s:sub(1,x-1) .. c .. s:sub(x+1) \n"
        "    niveau[#niveau-y+1] = s \n"
        "end \n"
        " \n"
        "function nbballs() \n"
        "    local result = 0 \n"
        "    for y=1,niveauH,1 do \n"
        "	for x=1,niveauL,1 do \n"
        "	    if getniveau(x,y) == \".\" then \n"
        "		result = result + 1 \n"
        "	    end \n"
        "	end \n"
        "    end \n"
        "    return result \n"
        "end \n"
        " \n"
        "niveauBalls = nbballs() \n"
        " \n"
        "function terrain() \n"
        "   pixBegin() \n"
        "   col(\"blue\") \n"
        "   for y=1,niveauH,1 do \n"
        "      for x=1,niveauL,1 do \n"
        "	  if getniveau(x,y) == \"*\" then \n"
        "	      pix(x,y)	 \n"
        "	  end \n"
        "      end \n"
        "   end \n"
        "   pixEnd() \n"
        "   GLUP.SetColor(GLUP.FRONT_COLOR,\"white\") \n"
        "   local Rgum = ((GLUP.ElapsedTime()*5)%2)*0.2+0.1 \n"
        "   GLUP.Begin(GLUP.SPHERES) \n"
        "   for y=1,niveauH,1 do \n"
        "      for x=1,niveauL,1 do \n"
        "          local c = getniveau(x,y) \n"
        "	  if c == \".\" then \n"
        "	      GLUP.Vertex(x+0.5,y+0.5,0.5,0.2) \n"
        "	  elseif c == \"O\" then \n"
        "	      GLUP.Vertex(x+0.5,y+0.5,0.5,Rgum)	 \n"
        "	  end \n"
        "      end \n"
        "   end \n"
        "   GLUP.End() \n"
        "end \n"
        " \n"
        "function GLUP.init_graphics() \n"
        "   GLUP.SetRegionOfInterest(1,1,1,niveauH,niveauH,1) \n"
        "end \n"
        " \n"
        "function imgui.on_key_pressed(key) \n"
        "   frozen = false \n"
        "   if key == \"left\" then \n"
        "      newvx=-1 \n"
        "      newvy=0 \n"
        "   elseif key == \"right\" then \n"
        "      newvx=1 \n"
        "      newvy=0 \n"
        "   elseif key == \"up\" then \n"
        "      newvx=0 \n"
        "      newvy=1 \n"
        "   elseif key == \"down\" then \n"
        "      newvx=0 \n"
        "      newvy=-1 \n"
        "   end \n"
        "end \n"
        " \n"
        "function dessinjoueur(x,y,vx,vy) \n"
        "    GLUP.Enable(GLUP.VERTEX_COLORS) \n"
        "    GLUP.Begin(GLUP.SPHERES) \n"
        "    col(\"yellow\") \n"
        "    GLUP.Vertex(x+0.5, y+0.5, 0.5, 0.5) \n"
        "    col(\"white\") \n"
        "    GLUP.Vertex(x+0.35+0.25*vx, y+0.5+0.25*vy, 1.0, 0.15) \n"
        "    GLUP.Vertex(x+0.65+0.25*vx, y+0.5+0.25*vy, 1.0, 0.15) \n"
        "    col(\"black\") \n"
        "    GLUP.Vertex(x+0.35+0.27*vx, y+0.5+0.27*vy, 1.1, 0.1) \n"
        "    GLUP.Vertex(x+0.65+0.27*vx, y+0.5+0.27*vy, 1.1, 0.1) \n"
        "    GLUP.End() \n"
        "    GLUP.Disable(GLUP.VERTEX_COLORS) \n"
        "end \n"
        " \n"
        "function dessinfantome(x,y,vx,vy,c) \n"
        "    GLUP.Enable(GLUP.VERTEX_COLORS) \n"
        "    GLUP.Begin(GLUP.SPHERES) \n"
        "    if not eatenmode then \n"
        "        if gummode then \n"
        "	   col(\"blue\") \n"
        "	else \n"
        "	   col(c) \n"
        "        end \n"
        "        GLUP.Vertex(x+0.5, y+0.5, 0.5, 0.5) \n"
        "        for xx=x+0.2,x+0.8,0.3 do \n"
        "            GLUP.Vertex(xx,y+0.1,0.5,0.3) \n"
        "        end \n"
        "    end \n"
        "    col(\"white\") \n"
        "    GLUP.Vertex(x+0.35+0.3*vx, y+0.5+0.3*vy, 1.0, 0.15) \n"
        "    GLUP.Vertex(x+0.65+0.3*vx, y+0.5+0.3*vy, 1.0, 0.15) \n"
        "    if eatenmode or not gummode then \n"
        "        col(\"black\") \n"
        "    	GLUP.Vertex(x+0.35+0.32*vx, y+0.5+0.32*vy, 1.1, 0.1) \n"
        "        GLUP.Vertex(x+0.65+0.32*vx, y+0.5+0.32*vy, 1.1, 0.1) \n"
        "    end \n"
        "    GLUP.End() \n"
        "    GLUP.Disable(GLUP.VERTEX_COLORS) \n"
        "end \n"
        " \n"
        "function killed() \n"
        "   frozen=true \n"
        "   joueurx=13 \n"
        "   joueury=10 \n"
        "   vx=0 \n"
        "   vy=0 \n"
        "   newvx=0 \n"
        "   newvy=0 \n"
        "   lasttime=0 \n"
        "   lastfantomtime=0 \n"
        " \n"
        "   gumtime=0 \n"
        "   gummode=false \n"
        "   eatentime=0 \n"
        "   eatenmode=false \n"
        " \n"
        "   fantomx=13 \n"
        "   fantomy=15 \n"
        "   fantomvx=0 \n"
        "   fantomvy=1 \n"
        "   lives = lives - 1 \n"
        "   if lives == 0 then \n"
        "       youlose() \n"
        "   end \n"
        "end \n"
        " \n"
        "function collision(time) \n"
        "   if joueurx == fantomx and joueury == fantomy then \n"
        "      if gummode then \n"
        "          score = score + 100 \n"
        "          eatenmode = true \n"
        "          eatentime = time \n"
        "       elseif not eatenmode then \n"
        "          killed() \n"
        "       end \n"
        "   end \n"
        "end \n"
        " \n"
        "function joueur() \n"
        " \n"
        "    local time = GLUP.ElapsedTime() \n"
        " \n"
        "    if gummode then \n"
        "	if (time - gumtime) > 5 then \n"
        "	    gummode = false \n"
        "	end \n"
        "    end \n"
        " \n"
        "    if not frozen and time-lasttime > 0.075 then \n"
        "	lasttime = time \n"
        "	local newx,newy \n"
        "	newx = joueurx+newvx \n"
        "	newy = joueury+newvy \n"
        "	if newx < 1 then \n"
        "	    newx = niveauL \n"
        "        elseif newx > niveauL then \n"
        "	    newx = 1 \n"
        "	end \n"
        "	if newy < 1 then \n"
        "	    newy = niveauH \n"
        "        elseif newy > niveauH then \n"
        "	    newy = 1 \n"
        "	end \n"
        "	local c = getniveau(newx,newy) \n"
        "	if c == \" \" or c == \".\" or c == \"O\" then \n"
        "	    joueurx = newx \n"
        "	    joueury = newy \n"
        "	    vx = newvx \n"
        "	    vy = newvy \n"
        "        else \n"
        "	    newx = joueurx + vx \n"
        "	    newy = joueury + vy \n"
        "	    c = getniveau(newx,newy) \n"
        "	    if c == \" \" or c == \".\" or c == \"O\" then \n"
        "	        joueurx = newx \n"
        "		joueury = newy \n"
        "	    end \n"
        "	end \n"
        "	if c == \"O\" then \n"
        "	    gumtime = time \n"
        "	    gummode = true \n"
        "	    fantomvx = -fantomvx \n"
        "	    fantomvy = -fantomvy \n"
        "	elseif c == \".\" then \n"
        "	    score = score + 10 \n"
        "	    niveauBalls = niveauBalls - 1 \n"
        "	    if niveauBalls == 0 then \n"
        "		youwin() \n"
        "	    end \n"
        "	end \n"
        "	setniveau(joueurx, joueury, \" \") \n"
        "    end \n"
        "    collision(time) \n"
        "    dessinjoueur(joueurx,joueury,vx,vy) \n"
        "end \n"
        " \n"
        "function fantome() \n"
        "    local time = GLUP.ElapsedTime() \n"
        "    local delay = 0.1 \n"
        "    if gummode then \n"
        "       delay = 0.2 \n"
        "    end \n"
        "    if eatenmode and (time - eatentime) > 5 then \n"
        "       eatenmode = false \n"
        "    end \n"
        "    if not frozen and time-lastfantomtime > delay then \n"
        "	lastfantomtime = time \n"
        "        while getniveau(fantomx + fantomvx, fantomy + fantomvy) ~= \" \" do \n"
        "	   local v = math.random(1,4) \n"
        "	   if v == 1 then \n"
        "	      fantomvx = -1 \n"
        "	      fantomvy = 0 \n"
        "	   elseif v == 2 then \n"
        "	      fantomvx = 1 \n"
        "	      fantomvy = 0 \n"
        "	   elseif v == 3 then \n"
        "	      fantomvx = 0 \n"
        "	      fantomvy = -1 \n"
        "	   elseif v == 4 then \n"
        "	      fantomvx = 0 \n"
        "	      fantomvy = 1 \n"
        "	   end \n"
        "	end \n"
        "	fantomx = fantomx + fantomvx \n"
        "        fantomy = fantomy + fantomvy \n"
        "    end \n"
        "    collision(time) \n"
        "    dessinfantome(fantomx,fantomy,fantomvx,fantomvy,\"pink\") \n"
        "end \n"
        " \n"
        "function GLUP.draw_scene() \n"
        "   GLUP.Enable(GLUP.DRAW_MESH) \n"
        "   GLUP.SetColor(GLUP.MESH_COLOR, 0,0,0.5) \n"
        "   GLUP.SetMeshWidth(2) \n"
        "   terrain() \n"
        "   if not gameover then \n"
        "       joueur() \n"
        "       fantome() \n"
        "   end \n"
        "end \n"
        " \n"
        "function imgui.draw_object_properties() \n"
        "   imgui.Text(\"Level: \" .. level) \n"
        "   imgui.Text(\"Score: \" .. score) \n"
        "   imgui.Text(\"Lives: \" .. lives) \n"
        "end \n"
     );

     register_embedded_lua_file("book/S01E01.lua",
        "-- FR Repare la fusee de Shift et Tab en utilisant pix et col \n"
        "-- EN Repair Shift and Tab's rocket by using pix and col \n"
        " \n"
        "import(\"pixel\") \n"
        " \n"
        "function GLUP.draw_scene() \n"
        " \n"
        "   -- FR Pour afficher les aretes des pixels \n"
        "   -- EN To display the edges of the pixels \n"
        "   GLUP.Enable(GLUP.DRAW_MESH) \n"
        " \n"
        "   -- FR Rend les pixels un peu plus petits \n"
        "   -- EN Makes the pixels a little bit smaller \n"
        "   GLUP.SetCellsShrink(0.1) \n"
        " \n"
        "   -- FR Affiche une grille pour mieux reperer les pixels \n"
        "   -- EN Displays a grid to help locating pixels \n"
        "   pixGrid() \n"
        " \n"
        "   pixBegin() \n"
        "   col(\"black\") \n"
        "   pix(1,1) \n"
        "   pix(1,2) \n"
        "   pix(3,1) \n"
        "   pix(3,2) \n"
        "   pix(5,2) \n"
        " \n"
        "   col(\"white\") \n"
        "   pix(2,2) \n"
        "   pix(4,2) \n"
        " \n"
        "   col(\"red\") \n"
        "   pix(2,3) \n"
        "   pix(3,4) \n"
        "   pix(2,5) \n"
        " \n"
        "   col(\"white\") \n"
        "   pix(3,3) \n"
        "   pix(2,4) \n"
        " \n"
        "   pixEnd() \n"
        "end \n"
     );

     register_embedded_lua_file("book/S01E02.lua",
        "-- FR Fais decoller la fusee de Shift et Tab \n"
        "-- EN Launch Shift and Tab's rocket \n"
        " \n"
        "import(\"pixel\") \n"
        " \n"
        "function GLUP.draw_scene() \n"
        " \n"
        "   -- FR Pour afficher les aretes des pixels \n"
        "   -- EN To display the edges of the pixels \n"
        "   GLUP.Enable(GLUP.DRAW_MESH) \n"
        " \n"
        "   -- FR Rend les pixels un peu plus petits \n"
        "   -- EN Makes the pixels a little bit smaller \n"
        "   GLUP.SetCellsShrink(0.1) \n"
        " \n"
        "   -- FR Affiche une grille pour mieux reperer les pixels \n"
        "   -- EN Displays a grid to help locating pixels \n"
        "   pixGrid() \n"
        " \n"
        "   pixBegin() \n"
        "   col(\"black\") \n"
        "   pix(1,1) \n"
        "   pix(1,2) \n"
        "   pix(3,1) \n"
        "   pix(3,2) \n"
        "   pix(5,1) \n"
        "   pix(5,2) \n"
        " \n"
        "   col(\"white\") \n"
        "   pix(2,2) \n"
        "   pix(4,2) \n"
        " \n"
        "   col(\"red\") \n"
        "   pix(2,3) \n"
        "   pix(4,3) \n"
        "   pix(3,4) \n"
        "   pix(2,5) \n"
        "   pix(4,5) \n"
        " \n"
        "   col(\"white\") \n"
        "   pix(3,3) \n"
        "   pix(2,4) \n"
        "   pix(4,4) \n"
        "   pix(3,5) \n"
        "   pix(2,6) \n"
        "   pix(3,6) \n"
        "   pix(4,6) \n"
        "   pix(3,7) \n"
        "   pix(3,8) \n"
        " \n"
        "   pixEnd() \n"
        "end \n"
     );

}
