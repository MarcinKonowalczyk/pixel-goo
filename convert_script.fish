
for f in *.png;
    echo $f;
    convert $f "conv_$f" && rm $f && mv "conv_$f" $f;
end

[ -f "00000.png" ] && rm "00000.png";

ffmpeg -y -framerate 60 \
# -start_number 1 \
-i %05d.png \
-vcodec libx264 -r 60 -pix_fmt yuv420p \
-color_range 2 -vf scale=in_range=full:out_range=full \
-movflags +write_colr \
# -frames:v 1 \
"./output.mp4"

exit(0)

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