BEGIN { file = "README.txt" }

/^File/ { file = $3; print file }
!/^File/ {print >> file}


