//+
SetFactory("OpenCASCADE");
//+
Point(1) = {-10.0, 0.0, 0, 1.0};
//+
Point(2) = {0.0, 0.0, 0, 1.0};
//+
Point(3) = {200.0, 0.0, 0, 1.0};
//+
Point(4) = {200.0, 100.0, 0, 1.0};
//+
Point(5) = {0.0, 100.0, 0, 1.0};
//+
Point(6) = {-10.0, 100.0, 0, 1.0};
//+
Point(7) = {-10.0, 0.5, 0, 1.0};
//+
Point(8) = {0.0, 0.5, 0, 1.0};
//+
Line(1) = {2, 3};
//+
Line(2) = {3, 4};
//+
Line(3) = {4, 5};
//+
Line(4) = {5, 6};
//+
Line(5) = {6, 7};
//+
Line(6) = {7, 8};
//+
Line(7) = {8, 2};
//+
Line(8) = {2, 1};
//+
Line(9) = {1, 7};
//+
Line(10) = {8, 5};

Curve Loop(1) = {3, -10, 7, 1, 2};
//+
Curve Loop(2) = {-10, -6, -5, -4};
Curve Loop(3) = {6, 7, 8, 9};

Plane Surface(1) = {1};
Plane Surface(2) = {2};
Plane Surface(3) = {3};
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
Physical Curve(7) = {7};
//+
Physical Curve(8) = {8};
//+
Physical Curve(9) = {9};
//+
Physical Surface(1) = {1, 2, 3};

Transfinite Surface {1} = {2, 3, 4, 5}; 

Transfinite Surface {2} = {7, 8, 5, 6}; 
//+
Transfinite Surface {3} = {1, 2, 8, 7}; 

Transfinite Curve {7, 9} = 65 Using Progression 1.0;
//+
Transfinite Curve {6, 8} = 33 Using Progression 1.0;
//+
Transfinite Curve {4, 6} = 33 Using Progression 1.0;
//+
Transfinite Curve {-5, 10} = 65 Using Progression 1.12163;
//+
Transfinite Curve {-3, 1} = 129 Using Progression 1.0211212;
//+
Transfinite Curve {2} = 129 Using Progression 1.0;
//+


Recombine Surface {1, 2, 3};
Mesh.MshFileVersion = 2.2;
//+

