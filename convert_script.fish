
# Convert from 
for f in *.png;
    echo $f;
    convert -flip $f "conv_$f" && rm $f && mv "conv_$f" $f;
end

# Delete the first frame if it exists
[ -f "00000.png" ] && rm "00000.png";

# Make a movie
ffmpeg -y -framerate 60 \
# -start_number 1 \
-i %05d.png \
-vcodec libx264 -r 60 -pix_fmt yuv420p \
-color_range 2 -vf scale=in_range=full:out_range=full \
-movflags +write_colr \
# -frames:v 1 \
"./output.mp4"

# Also make a gif
# Take every other 
convert -delay 3 -flip -verbose (ls -1 *{0,2,4,6,8}.png) "output.gif"
# gifsicle -O1 --lossy=80 output.gif "#0-1200" -o web-output.gif

exit 0

# convert -verbose -delay 6 -quality 95 *.png "output.gif";

ffmpeg -y -framerate 60 -start_number 1 -i %05d.bmp \
-vcodec libx264 -r 60 -pix_fmt yuv420p \
-color_range 2 -vf scale=in_range=full:out_range=full \
-movflags +write_colr \
# -frames:v 1 \
"./output.mp4"


ffmpeg -y -framerate 60 \
# -start_number 1 \
-f image2 \
-i %05d.png \
-r 60 \
-c:v libx264rgb \
-crf 1 \
# -frames:v 1 \
"./output.mp4"

ffmpeg -y -framerate 60 \
# -start_number 1 \
# -f image2 \
-i %05d.png \
-r 60 \
-c:v libx264rgb \
# -crf 1 \
# -frames:v 1 \
"./output.mkv"