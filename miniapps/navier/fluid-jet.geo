// Gmsh project created on Tue Nov 29 20:54:58 2022
SetFactory("OpenCASCADE");
 
f=1e-2;

Point(1) = {0.0, 0.0, 0, f};
Point(2) = {2.0, 0.0, 0, f};Point(3) = {2.0, 1.0, 0, f};
Point(4) = {0.0, 1.0, 0, f};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 4};
Line(4) = {4, 1};



//+

Curve Loop(1) = {1, 2, 3, 4};
//+
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
Physical Surface(1) = {1};
//+


// https://mfem.org/mesh-formats/
//+
Recombine Surface{1};
Mesh.MshFileVersion = 2.2;