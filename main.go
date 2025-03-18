package main

import (
	rl "github.com/gen2brain/raylib-go/raylib"
)

const (
	simWidth       int32   = 1500
	simHeight      int32   = 900
	windowWidth    int32   = simWidth
	windowHeight   int32   = simHeight + 80
	targetFPS      int32   = 60
	fixedDeltaTime float32 = 1 / float32(targetFPS)
	dampingFactor  float32 = 0.9 // Todo: Remove this and implement proper collision with energy loss
	// pixelsPerMeter int32 = 20
)

type SimObject interface {
	update()
	draw()
	isDynamic() bool
	isClicked() bool
}

type Simulation struct {
	objects []SimObject
	// graphs          []XYGraph
	buttons         map[string]*TextButton
	runSim          bool
	accumulatedTime float32
	// metric          bool
	// forces          bool
	// paused          bool
	// siUnit          bool
	// showGrid        bool
	// accumulatedTime float32
}

var colors []rl.Color = []rl.Color{rl.Yellow, rl.Pink, rl.Red, rl.Beige, rl.SkyBlue}

func NewSimulation() *Simulation {
	sim := &Simulation{
		buttons: make(map[string]*TextButton),
	}

	// Initialize graphs
	// for _, c := range sim.circles {
	// 	sim.graphs = append(sim.graphs, XYGraph{
	// 		origin: rl.NewVector2(400, 400),
	// 		points: []rl.Vector2{},
	// 		color:  c.col,
	// 	})
	// }

	// Initialize buttons
	buttonNames := []string{"Add", "Run"}
	xPos := float32(20)
	for _, name := range buttonNames {
		sim.buttons[name] = &TextButton{
			pos:  rl.NewVector2(xPos, float32(simHeight)+20),
			text: name,
		}
		xPos += 80 // Spacing
	}

	return sim
}

func (sim *Simulation) Run() {
	for !rl.WindowShouldClose() {
		sim.accumulatedTime += rl.GetFrameTime()
		for sim.accumulatedTime >= fixedDeltaTime {
			sim.updatePhysics()
			sim.accumulatedTime -= fixedDeltaTime
		}
		sim.updatePhysics()
		sim.handleInput()
		sim.render()
	}
}

func (sim *Simulation) updatePhysics() {
	if !sim.runSim {
		return
	}

	for i := range sim.objects {
		sim.objects[i].update()
	}

	// for i := range sim.circles {
	// 	sim.circles[i].update()
	// if len(sim.graphs[i].points) < 1000 {
	// 	sim.graphs[i].points = append(sim.graphs[i].points, sim.circles[i].pos)
	// }
	// }
}

func (sim *Simulation) handleInput() {

	// if sim.buttons["Metric"].isClicked() {
	// 	sim.metric = !sim.metric
	// }
	// if sim.buttons["Forces"].isClicked() {
	// 	sim.forces = !sim.forces
	// }
	// if sim.buttons["Pause"].isClicked() {
	// 	sim.togglePause()
	// }
	// if sim.buttons["SI"].isClicked() {
	// 	sim.siUnit = !sim.siUnit
	// }
	if sim.buttons["Run"].isClicked() {
		sim.runSim = !sim.runSim
	}
	if sim.buttons["Add"].isClicked() {
		sim.objects = append(sim.objects, &Circle{
			pos:    rl.NewVector2(400, 400),
			radius: float32(10),
			vel:    rl.NewVector2(20, -700),
			acc:    rl.NewVector2(0, 700),
			col:    colors[len(sim.objects)%len(colors)],
		})
	}
}

func (sim *Simulation) render() {
	rl.BeginDrawing()
	rl.ClearBackground(rl.Black)

	// Draw UI
	rl.DrawLine(0, simHeight, windowWidth, simHeight, rl.Gray)
	for _, btn := range sim.buttons {
		btn.draw()
	}

	// Draw objects
	for _, obj := range sim.objects {
		obj.draw()
	}

	// Draw graphs
	// for _, g := range sim.graphs {
	// 	g.draw()
	// }

	// Draw additional elements
	// sim.drawMetric()
	// sim.showForces()
	// applyAccAtMousePos()

	rl.EndDrawing()
}

// func (sim *Simulation) drawMetric() {
// 	if !sim.metric {
// 		return
// 	}

// 	y := int32(20)
// 	for i, c := range sim.primaryCircles {
// 		posX, posY := c.pos.X, c.pos.Y
// 		velX, velY := c.vel.X, c.vel.Y

// 		unit := "p/s"
// 		if sim.siUnit {
// 			posX, posY = pixelToSI(posX), pixelToSI(posY)
// 			velX, velY = pixelToSI(velX), pixelToSI(velY)
// 			unit = "m/s"
// 		}

// 		rl.DrawText(fmt.Sprintf("Circle %d: (%.2f, %.2f) %s", i+1, posX, posY, unit), 20, y, 10, rl.Gray)
// 		y += 20
// 	}
// }

// Helper functions
// func pixelToSI(pixel float32) float32 { return pixel / float32(pixelsPerMeter) }
// func toPixel(si float32) float32      { return si * float32(pixelsPerMeter) }

func main() {
	rl.InitWindow(windowWidth, windowHeight, "Physics Simulations in Golang")
	defer rl.CloseWindow()
	rl.SetTargetFPS(targetFPS)

	// Initialize simulation
	sim := NewSimulation()
	sim.Run()

	// drawMetric()
	// showForces()

	// applyAccAtMousePos()
}

// func applyAccAtMousePos() {
// 	mousePos := rl.GetMousePosition()
// 	if rl.IsMouseButtonDown(rl.MouseLeftButton) && mouseInSpace() && !pause {
// 		for i := range circles {
// 			circles[i].acc = rl.Vector2Scale(rl.Vector2Normalize(rl.Vector2Subtract(mousePos, circles[i].pos)), 700)
// 		}
// 	} else {
// 		if !pause {
// 			for i := range circles {
// 				circles[i].acc = rl.NewVector2(0, 700)
// 			}
// 		}
// 	}
// }

// func drawMetric() {
// 	var y int32 = 20
// 	var n int32 = 1
// 	if metric {
// 		for _, c := range primaryCircles {
// 			posX := c.pos.X
// 			posY := c.pos.Y
// 			velX := c.vel.X
// 			velY := c.vel.Y
// 			accX := c.acc.X
// 			accY := c.acc.Y
// 			unit := "p/s"

// 			if siUnit {
// 				posX = pixelToSI(posX)
// 				posY = pixelToSI(posY)
// 				velX = pixelToSI(velX)
// 				velY = pixelToSI(velY)
// 				accX = pixelToSI(accX)
// 				accY = pixelToSI(accY)
// 				unit = "m/s"
// 			}

// 			// position
// 			rl.DrawText(fmt.Sprintf("Position %d: (%.2f, %.2f)", n, posX, posY), 20, y, 10, rl.Gray)
// 			rl.DrawCircle(10, y+5, 5, c.col)

// 			// velocity
// 			rl.DrawText(fmt.Sprintf("Velocity %d: (%.2f, %.2f) %s", n, velX, velY, unit), 220, y, 10, rl.Gray)
// 			rl.DrawCircle(210, y+5, 5, c.col)

// 			// acceleration
// 			rl.DrawText(fmt.Sprintf("Acceleration %d: (%.2f, %.2f) %s", n, accX, accY, unit), 420, y, 10, rl.Gray)
// 			rl.DrawCircle(410, y+5, 5, c.col)

// 			y += 20
// 			n += 1
// 		}

// 		// mouse
// 		mousePos := rl.GetMousePosition()
// 		rl.DrawText(fmt.Sprintf("Mouse: %f, %f", mousePos.X, mousePos.Y), 620, 20, 10, rl.Gray)
// 	}
// }

// func showForces() {
// 	if forces {
// 		for _, c := range circles {
// 			arrow := Arrow{
// 				rl.NewVector2(c.pos.X, c.pos.Y),
// 				rl.Vector2Add(c.pos, rl.Vector2Scale(rl.Vector2Normalize(c.acc), 100)),
// 			}
// 			arrow.draw()
// 		}
// 	}
// }

// var circlesSec []Circle

// func playPause() {
// 	if pause {
// 		circlesSec = make([]Circle, len(circles))
// 		copy(circlesSec, circles)
// 		for i := range circles {
// 			circles[i].vel = rl.NewVector2(0, 0)
// 			circles[i].acc = rl.NewVector2(0, 0)
// 		}
// 		primaryCircles = circlesSec
// 	} else {
// 		for i := range circles {
// 			circles[i].vel = circlesSec[i].vel
// 			circles[i].acc = circlesSec[i].acc
// 		}
// 		primaryCircles = circles
// 	}
// }

// // 1 meter = 20px
// func pixelToSI(pixel float32) float32 {
// 	return pixel / float32(pixelPerMetre)
// }

// func toPixel(si float32) float32 {
// 	return si * float32(pixelPerMetre)
// }
