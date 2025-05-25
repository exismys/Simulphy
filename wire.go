package main

import rl "github.com/gen2brain/raylib-go/raylib"

type Wire struct {
	From     rl.Vector2
	To       rl.Vector2
	color    rl.Color
	FromPort *Port
	ToPort   *Port
}

func (w *Wire) draw(cameraOffset *rl.Vector2) {
	posFrom := rl.Vector2Subtract(w.From, *cameraOffset)
	posTo := rl.Vector2Subtract(w.To, *cameraOffset)
	rl.DrawLineV(posFrom, posTo, rl.Gray)
}

func (w *Wire) update() {
}

func (w *Wire) isDynamic() bool {
	return false
}

func (w *Wire) setPosition(position rl.Vector2) {
	w.To = position
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
