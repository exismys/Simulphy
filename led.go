package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type Led struct {
	pos          rl.Vector2
	radius       float32
	color        rl.Color
	state        bool
	inputPort    *Port
	cameraOffset *rl.Vector2
}

func NewLed(sim *Simulation, position rl.Vector2) *Led {
	l := &Led{
		pos:    position,
		radius: 20,
		state:  false,
		color:  rl.Gray,
	}
	l.inputPort = &Port{
		pos:       rl.NewVector2(position.X+l.radius+6, position.Y),
		radius:    5,
		inputPort: true,
		color:     rl.SkyBlue,
		fromPorts: []*Port{},
	}
	leds = append(leds, l)
	l.inputPort.onClick = func() {
		fmt.Println("Input port of the Led clicked!")
		if sim.ghostObject != nil {
			w := sim.ghostObject.(*Wire)
			w.To = l.inputPort.pos
			w.From = rl.Vector2Add(w.From, sim.cameraOffset)
			sim.objects = append(sim.objects, w)
			wires = append(wires, w)
			l.inputPort.fromPorts = append(l.inputPort.fromPorts, w.FromPort)
			finalPort = l.inputPort
			l.state = calculateState(finalPort)
			fmt.Println("Led State: ", l.state)
			fmt.Println("Number of wires: ", len(wires))
			sim.ghostObject = nil
		}
	}
	return l
}

func (l *Led) draw(cameraOffset *rl.Vector2) {
	l.cameraOffset = cameraOffset
	color := l.color
	if l.state {
		color = rl.Red
	}
	rl.DrawCircleV(rl.Vector2Subtract(l.pos, *cameraOffset), l.radius, color)
	l.inputPort.draw(cameraOffset)
}

func (l *Led) update() {
}

func (l *Led) HandleInput() {
	l.inputPort.HandleInput()
}

func (l *Led) setTranslucent(set bool) {
	if set {
		l.color.A = 128
		l.inputPort.color.A = 128
	} else {
		l.color.A = 255
		l.inputPort.color.A = 255
	}
}

func (l *Led) isDynamic() bool {
	return false
}

func (l *Led) setPosition(position rl.Vector2) {
	l.pos = position
	l.inputPort.pos = rl.NewVector2(position.X-l.radius-6, position.Y)
}

func (l *Led) hovered() bool {
	return false
}
