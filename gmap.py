from gmplot import gmplot

# Initialize the map at a given point
# Lat,lon,API
gmap = gmplot.GoogleMapPlotter(1.3521, 103.8198, 13)

# Add a marker
#:at, Lon, Colour
gmap.marker(1.3777444453128973, 103.84876553996334, 'cornflowerblue')

# Draw map into HTML file
gmap.draw("my_map.html")