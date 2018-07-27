outerDimensions = [15, 10, 4];
wallThickness = 0.04;
materialColor = [0.2, 0.3, 0.3];

difference() {
    color(materialColor) cube(outerDimensions);
    w = 2*wallThickness;
    translate([wallThickness, wallThickness, wallThickness]) {
        color(materialColor) cube([outerDimensions [0]- w, outerDimensions[1] - w, outerDimensions[2]]);
    }
}
