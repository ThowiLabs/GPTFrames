#pragma once
// Escenarios cinéticos de demostración: geometría y animación nativas.
static void demoStar(int cx,int cy,int r,int points,double angle,RGB ink,float alpha=1){
 for(int i=0;i<points*2;i++){
  double a=angle+(PI*i)/points-PI/2, b=angle+(PI*(i+1))/points-PI/2;
  int ra=(i%2==0?r:int(r*.40)),rb=((i+1)%2==0?r:int(r*.40));
  line(cx+int(ra*cos(a)),cy+int(ra*sin(a)),cx+int(rb*cos(b)),cy+int(rb*sin(b)),ink,alpha,2);
 }
}
static void demoGridCube(int cx,int cy,int rw,int rh,int depth,RGB ink,float a){
 int x=cx-rw/2,y=cy-rh/2;
 for(int i=0;i<4;i++){
  int ax=x+(i%2)*rw,ay=y+(i/2)*rh;
  line(ax,ay,ax+depth,ay-depth/2,ink,a,2);
 }
 for(int j=0;j<2;j++){
  line(x+depth,y+depth*(-.5)+j*rh,x+depth+rw,y+depth*(-.5)+j*rh,ink,a,2);
  line(x+depth+j*rw,y-depth/2,x+depth+j*rw,y+rh-depth/2,ink,a,2);
 }
 for(int j=0;j<2;j++){
  line(x,y+j*rh,x+rw,y+j*rh,ink,a,2);
  line(x+j*rw,y,x+j*rw,y+rh,ink,a,2);
 }
}
static void showcaseEffect(FontPainter& font,const Scene& s,int index,double local,int frame){
 const int mode=index-6;
 double t=std::max(0.,local),time=t/24.;
 float appear=reveal(t,20,28);
 RGB a=s.accent,b=s.secondary;
 auto text=[&](int x,int y,const std::string& val,int px,RGB c,float visible=1.f,int maxw=590){
   if(visible>0)label(font,x,y,val,px,c,visible,maxw);
 };
 if(mode==0){ // animated emphasis / rough-like notation
   card(62,777,594,271,25,a,s.deep,appear);
   const char* fragments[]={"SPRING","INTERPOLATE","HIGHLIGHT"};
   for(int i=0;i<3;i++){
     float k=springAt(t,27+i*13,.115);
     int yy=799+i*76;
     int xx=90+int((1-k)*370);
     if(i==1)roundRect(xx-9,yy-1,int(430*reveal(t,35,29)),58,10,a,.7f);
     text(xx,yy,fragments[i],i==2?43:50,i==1?black:(i==0?white:b),appear,515);
     if(i==2)remix::rect(xx,yy+58,int(510*reveal(t,52,37)),4,a,appear);
   }
   text(89,1055,"ECUACIONES → MOVIMIENTO",19,a,appear,550);
 }else if(mode==1){ // layered parallax panels
   for(int i=3;i>=0;i--){
     int dx=int(sin(time*(i*.39+.55)+i)*37+(i-1)*12);
     int dy=int(cos(time*(i*.35+.40)+i)*18);
     int x=90+dx+i*21,y=793+dy+i*34;
     card(x,y,490-i*25,185,22,i%2?a:b,s.deep,appear*(1-i*.08f));
     remix::rect(x+26,y+36,70,5,i%2?a:b,appear);
     text(x+28,y+58,(i==0?"PRIMER PLANO":i==1?"CAPA MEDIA":i==2?"PROFUNDIDAD":"BACKGROUND"),i==0?37:25,white,appear,400);
     if(i==0){text(x+30,y+109,"Z = 4.0",23,a,appear,200);}
   }
   text(83,1080,"MOVIMIENTO A DIFERENTES VELOCIDADES",18,a,appear,550);
 }else if(mode==2){ // graphs, counters and dynamic values
   card(67,759,583,335,24,a,s.deep,appear);
   int value=int(1200*reveal(t,38,95));
   text(98,791,"ANALÍTICA EN TIEMPO REAL",20,a,appear,500);
   text(99,832,std::to_string(value)+"  /  1200",46,white,appear,505);
   for(int i=0;i<10;i++){
     int x=95+i*52;
     float h=.32f+.68f*float(std::abs(sin(i*.82+time*.47)))*reveal(t,35+i*4,55);
     int height=int(165*h);
     roundRect(x,1070-height,32,height,7,i%3==0?b:a,appear);
   }
   for(int i=0;i<9;i++)line(95+i*52,1085,104+i*52,1085,white,.35f);

 }else if(mode==3){ // iris and textured wipes within a panel
   card(69,770,583,333,22,a,s.deep,appear);
   float p=reveal(t,35,110);
   for(int j=0;j<5;j++){
     float prog=reveal(t,29+j*13,48);
     int x=98+(j%3)*171,y=814+(j/3)*112;
     roundRect(x,y,151,91,12,{19,40,67},appear);
     if(j%2==0)roundRect(x,y,int(151*prog),91,11,j%4==0?a:b,appear*.82f);
     else for(int q=0;q<11;q++){
       float dist=std::abs(q-5)/5.f;
       roundRect(x+q*13,y+10+(int)(dist*8),10,int(68*prog),4,b,appear);
     }
   }
   text(98,1061,"IRIS / WIPE / DISSOLVE",29,white,appear,520);
   remix::rect(98,1114,int(524*p),5,a);
 }else if(mode==4){ // vector drawing : geometry / rotation / primitives
   float rotation=time*.9f;
   card(67,766,585,322,22,a,s.deep,appear);
   ring(235,901,104,a,appear,3,rotation,true);
   ring(235,901,65,b,appear,2,-rotation*2,true);
   demoStar(235,900,int(82*springAt(t,30,.11)),6,rotation,a,appear);
   demoGridCube(479,894,132,125,50,b,appear);
   line(95,1044,int(mix(95,617,reveal(t,28,90))),1044,a,appear,4);
   text(97,1062,"FORMAS / SVG / TRAYECTORIAS",21,white,appear,520);
 }else if(mode==5){ // additional field attractors / trajectories
   card(66,758,586,338,21,a,s.deep,appear);
   int ox=361,oy=914;
   for(int j=0;j<7;j++){
     ring(ox,oy,42+j*24,a,appear*.23f,2,time*(j%2?1:-1),j%3==0);
   }
   for(int i=0;i<105;i++){
     double z=(i*2.39996323)+time*(.27+(i%9)*.022);
     double radius=16+((i*137+frame*4)%250);
     int x=ox+int(cos(z)*radius),y=oy+int(sin(z)*radius*.68);
     roundRect(x,y,i%6==0?7:3,i%6==0?7:3,2,i%3?b:a,appear*.86f);
   }
   text(98,1070,"PARTÍCULAS DETERMINISTAS",22,a,appear,530);
 }else if(mode==6){ // 3D wireframe and perspective fake cards
   float swing=sin(time*1.2f);
   for(int layer=4;layer>=0;layer--){
     int depth=layer*35,dx=int(swing*(layer*21));
     demoGridCube(359+dx,888-depth/3,350-layer*45,215-layer*14,90-layer*11,layer%2?a:b,appear*(.95f-layer*.12f));
   }
   int cx=332+int(swing*45),cy=813+int(sin(time*.8)*12);
   card(cx-225,cy,438,230,24,a,s.deep,appear);
   text(cx-190,cy+33,"3D CAMERA",46,white,appear,394);
   text(cx-187,cy+111,"PERSPECTIVA",32,a,appear,375);
   text(103,1071,"ROTACIÓN + MOVIMIENTO Z",22,a,appear,530);
 }else if(mode==7){ // native procedural audio spectrum
   card(68,762,586,342,22,a,s.deep,appear);
   text(98,792,"AUDIO VISUALIZER",35,white,appear,520);
   for(int i=0;i<40;i++){
     double amp=.14+.86*pow(std::abs(sin(time*3.3+i*.31)*cos(time*1.8+i*.075)),1.5);
     int height=int(17+amp*202*reveal(t,22+i*.5,38));
     int x=94+i*13;
     roundRect(x,1052-height,8,height,3,i%4==0?b:a,appear);
   }
   line(97,1060,615,1060,a,.6f);
   text(97,1070,"SÍNTESIS + REACTIVIDAD",21,a,appear,505);
 }else{ // grand finale montage: scene and transitions info
   card(65,766,590,352,23,a,s.deep,appear);
   static const char* titles[]={"SEQUENCE","SPRING","MASK","SHAPES","CAMERA","AUDIO","TRANSITIONS"};
   for(int i=0;i<7;i++){
     int x=95+(i%2)*281,y=796+(i/2)*74;
     float revealItem=reveal(t,16+i*8,27);
     int move=int((1-revealItem)*(i%2?24:-24));
     roundRect(x+move,y,257,56,12,i%2?a:b,appear*revealItem*.25f);
     text(x+13+move,y+15,titles[i],24,white,appear*revealItem,236);
   }
   text(99,1045,"HECHO EN C++17",30,a,appear,520);
 }
}
