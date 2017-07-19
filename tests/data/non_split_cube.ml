GOCAD Model3d 1 
HEADER {
name:non_split_cube
}
GOCAD_ORIGINAL_COORDINATE_SYSTEM
NAME Default
AXIS_NAME "X" "Y" "Z"
AXIS_UNIT "m" "m" "m"
ZPOSITIVE Depth
END_ORIGINAL_COORDINATE_SYSTEM
TSURF box
TSURF horizon
TFACE 1  boundary box
  0.0 0.0 0.0
  1.0 0.0 0.0
  1.0 1.0 0.0
TFACE 2  top horizon
  0.0 0.0 0.3
  0.6 0.0 0.3
  0.6 1.0 0.3
REGION 3 Universe 
  -1   0
REGION 4  Top 
  +1  +2  -2   0
END
GOCAD TSurf 1 
HEADER {
name:box
name_in_model_list:box
}
GOCAD_ORIGINAL_COORDINATE_SYSTEM
NAME Default
AXIS_NAME "X" "Y" "Z"
AXIS_UNIT "m" "m" "m"
ZPOSITIVE Depth
END_ORIGINAL_COORDINATE_SYSTEM
GEOLOGICAL_FEATURE box
GEOLOGICAL_TYPE boundary
PROPERTY_CLASS_HEADER Z {
is_z:on
}
TFACE
VRTX 1 0.0 0.0 0.0 
VRTX 2 1.0 0.0 0.0 
VRTX 3 1.0 1.0 0.0 
VRTX 4 0.0 1.0 0.0
VRTX 5 0.0 0.0 0.3 
VRTX 6 0.6 0.0 0.3 
VRTX 7 0.6 1.0 0.3 
VRTX 8 0.0 1.0 0.3
VRTX 9 0.0 0.0 1.0
VRTX 10 1.0 0.0 1.0 
VRTX 11 1.0 1.0 1.0 
VRTX 12 0.0 1.0 1.0
ATOM 13 5 
ATOM 14 8
TRGL 1 2 3 
TRGL 1 3 4 
TRGL 1 6 2
TRGL 1 5 6
TRGL 2 6 10
TRGL 13 10 6 
TRGL 13 9 10
TRGL 2 11 3 
TRGL 2 10 11
TRGL 3 8 4 
TRGL 3 7 8
TRGL 3 11 7
TRGL 7 11 12
TRGL 7 12 14 
TRGL 4 5 1
TRGL 4 8 5   
TRGL 14 12 9 
TRGL 14 9 13 
TRGL 9 10 11
TRGL 9 11 12  
BSTONE 6
BSTONE 7
BORDER 13 6 5
BORDER 14 6 13
END
GOCAD TSurf 1 
HEADER {
name:horizon
name_in_model_list:horizon
}
GOCAD_ORIGINAL_COORDINATE_SYSTEM
NAME Default
AXIS_NAME "X" "Y" "Z"
AXIS_UNIT "m" "m" "m"
ZPOSITIVE Depth
END_ORIGINAL_COORDINATE_SYSTEM
PROPERTY_CLASS_HEADER Z {
is_z:on
}
TFACE
VRTX 1 0.0 0.0 0.3 
VRTX 2 0.6 0.0 0.3 
VRTX 3 0.6 1.0 0.3 
VRTX 4 0.0 1.0 0.3
TRGL 1 2 3 
TRGL 1 3 4 
BSTONE 2 
BSTONE 3
BORDER 5 2 3
BORDER 6 2 1 
END