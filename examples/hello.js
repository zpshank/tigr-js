import * as tigr from 'tigr';

const w = new tigr.Window(320, 240, "Hello!", 0);

while(!w.closed()) {
	w.clear(tigr.rgb(0x80, 0x90, 0xa0));
	w.print(tigr.TFONT, 120, 110, tigr.rgb(0xff, 0xff, 0xff), "Hello, world.");
	w.update();
}
