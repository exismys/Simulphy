package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Wire struct {
	From  *Port
	To    *Port
	color rl.Color
}

func (w *Wire) draw(cameraOffset *rl.Vector2) {
	rl.DrawLineV(w.From.pos, w.To.pos, rl.Gray)
}

func (w *Wire) update() {
}

func (w *Wire) isDynamic() bool {
	return false
}

func (w *Wire) setPosition(position rl.Vector2) {
	w.To = &Port{
		pos: position,
	}
}

func (w *Wire) setTranslucent(set bool) {
	if set {
		w.color.A = 128
	} else {
		w.color.A = 255
	}
}

func (w *Wire) HandleInput() {
}
