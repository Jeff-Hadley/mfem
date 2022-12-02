//+
SetFactory("OpenCASCADE");

//+
Point(1) = {0.0, -100.0, 0, 1.0};
//+
Point(2) = {200.0, -100.0, 0, 1.0};
//+
Point(3) = {200.0, 100.0, 0, 1.0};
//+
Point(4) = {0.0, 100.0, 0, 1.0};
//+
Point(5) = {0.0, 0.5, 0, 1.0};
Point(6) = {0.0, -0.5, 0, 1.0};

Line(1) = {1, 2};
//+
Line(2) = {2, 3};
//+
Line(3) = {3, 4};
//+
Line(4) = {4, 5};
//+
Line(5) = {5, 6};
//+
Line(6) = {6, 1};

Curve Loop(1) = {1, 2, 3, 4, 5, 6};

Plane Surface(1) = {1};
//+
Physical Curve(1) = {1};
//+
Physical Curve(2) = {2};
//+
Physical Curve(3) = {3};
//+
Physical Curve(4) = {4};
//+
Physical Curve(5) = {5};
//+
Physical Curve(6) = {6};
//+
Physical Surface(1) = {1};


Transfinite Curve {5} = 129 Using Progression 1.0;
//+
Transfinite Curve {-4, 6} = 65 Using Progression 1.12163;
//+
Transfinite Curve {-3, 1} = 129 Using Progression 1.0211212;
//+
Transfinite Curve {2} = 257 Using Progression 1.0;
//+


Recombine Surface {1};
Mesh.MshFileVersion = 2.2;
//+

//+
Transfinite Surface {1} = {1, 2, 3, 4};
