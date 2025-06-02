package main

import (
	"fmt"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type Led struct {
	Pos          rl.Vector2
	Radius       float32
	Color        rl.Color
	State        bool
	InputPort    *Port
	InputPortId  int32
	CameraOffset rl.Vector2
}

func NewLed(sim *Simulation, position rl.Vector2) *Led {
	l := &Led{
		Pos:    position,
		Radius: 20,
		State:  false,
		Color:  rl.Gray,
	}
	l.InputPort = &Port{
		Id:          getNewPortId(),
		Pos:         rl.NewVector2(position.X+l.Radius+6, position.Y),
		Radius:      5,
		IsInputPort: true,
		Color:       rl.SkyBlue,
	}
	l.InputPort.onClick = func() {
		fmt.Println("Input port of the Led clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = l.InputPort.Pos
			w.ToPort = l.InputPort
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			sim.Objects = append(sim.Objects, w)
			wires = append(wires, w)
			l.InputPort.FromPorts = append(l.InputPort.FromPorts, w.FromPort)
			l.InputPort.FromPortsIds = append(l.InputPort.FromPortsIds, w.FromPort.Id)
			finalPort = l.InputPort
			l.State = calculateState(finalPort)
			fmt.Println("Led State: ", l.State)
			fmt.Println("Number of wires: ", len(wires))
			sim.GhostObject = nil
		}
	}
	l.InputPortId = l.InputPort.Id
	portMap[l.InputPort.Id] = l.InputPort
	fmt.Println(portMap)
	return l
}

func (l *Led) draw(cameraOffset *rl.Vector2) {
	l.CameraOffset = *cameraOffset
	color := l.Color
	if l.State {
		color = rl.Red
	}
	rl.DrawCircleV(rl.Vector2Subtract(l.Pos, *cameraOffset), l.Radius, color)
	l.InputPort.draw(cameraOffset)
}

func (l *Led) update() {
}

func (l *Led) HandleInput() {
	l.InputPort.HandleInput()
	if l.hovered() && rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
		fmt.Printf("Address of the input port (ID: %d): %p\n", l.InputPortId, l.InputPort)
	}
}

func (l *Led) setTranslucent(set bool) {
	if set {
		l.Color.A = 128
		l.InputPort.Color.A = 128
	} else {
		l.Color.A = 255
		l.InputPort.Color.A = 255
	}
}

func (l *Led) isDynamic() bool {
	return false
}

func (l *Led) setPosition(position rl.Vector2) {
	l.Pos = position
	l.InputPort.Pos = rl.NewVector2(position.X-l.Radius-6, position.Y)
}

func (l *Led) hovered() bool {
	mouse := rl.GetMousePosition()
	return rl.CheckCollisionPointCircle(mouse, rl.Vector2Subtract(l.Pos, l.CameraOffset), l.Radius)
}
