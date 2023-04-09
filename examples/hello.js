import {Window} from 'tigr';

const w = new Window(320, 240, "Hello!", 0);

while(!w.closed()) {
	w.update();
}
