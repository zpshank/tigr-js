import * as tigr from 'tigr';

const w = new tigr.Window(800, 600, "Hello!", 0);

while(!w.closed()) {
	w.clear(tigr.rgb(0x80, 0x90, 0xa0));
	w.update();
}
