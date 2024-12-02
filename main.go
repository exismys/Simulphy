package main

import (
	"fmt"
	"math/rand/v2"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type Circle struct {
	pos    rl.Vector2
	radius float32
	vel    rl.Vector2
	acc    rl.Vector2
	col    rl.Color
}

const spaceWidth int32 = 1500
const spaceHeight int32 = 900
const windowWidth int32 = spaceWidth
const windowHeight int32 = spaceHeight + 80

const pixelPerMetre int32 = 20

const targetFPS int32 = 60
const fixedDeltaTime float32 = 1 / float32(targetFPS)

const dumpingFactor float32 = 0.9

const numberOfCircle int32 = 5
const radius int32 = 10

var circles []Circle
var primaryCircles []Circle
var metric bool = true
var forces bool = false
var pause bool = false
var siUnit bool = true
var colors []rl.Color = []rl.Color{rl.Yellow, rl.Pink, rl.Red, rl.Beige, rl.SkyBlue}

func main() {

	rl.InitWindow(windowWidth, windowHeight, "Physics Simulations in Golang")
	defer rl.CloseWindow()

	rl.SetTargetFPS(targetFPS)

	circles = make([]Circle, numberOfCircle)
	for i := 0; i < len(circles); i++ {
		x := rand.Float32() * float32(spaceWidth)
		circles[i] = Circle{
			pos:    rl.NewVector2(x, 400),
			radius: float32(radius),
			vel:    rl.NewVector2(20, -700),
			acc:    rl.NewVector2(0, 700),
			col:    colors[i],
		}
	}
	primaryCircles = circles

	buttonMetric := TextButton{
		pos:  rl.NewVector2(20, float32(spaceHeight)+20),
		text: "",
	}

	buttonForce := TextButton{
		pos:  rl.NewVector2(20+60+20, float32(spaceHeight)+20),
		text: "",
	}

	buttonPause := TextButton{
		pos:  rl.NewVector2(20+60+20+60+20, float32(spaceHeight)+20),
		text: "",
	}

	buttonSI := TextButton{
		pos:  rl.NewVector2(20+60+20+60+20+60+20, float32(spaceHeight)+20),
		text: "",
	}

	grid := Grid{
		rl.NewVector2(400, 400),
		rl.DarkGray,
	}

	accumulatedTime := float32(0)
	for !rl.WindowShouldClose() {
		accumulatedTime += rl.GetFrameTime()
		for accumulatedTime >= fixedDeltaTime {
			for i := range circles {
				circles[i].move()
			}
			accumulatedTime -= fixedDeltaTime
		}
		rl.BeginDrawing()

		rl.DrawLine(0, spaceHeight, windowWidth, spaceHeight, rl.Gray)

		if buttonMetric.isClicked() {
			metric = !metric
		}
		if metric {
			buttonMetric.text = "! metric"
		} else {
			buttonMetric.text = "metric"
		}
		buttonMetric.draw()

		if buttonForce.isClicked() {
			forces = !forces
		}
		if forces {
			buttonForce.text = "! forces"
		} else {
			buttonForce.text = "forces"
		}
		buttonForce.draw()

		if buttonPause.isClicked() {
			pause = !pause
			playPause()
		}
		if pause {
			buttonPause.text = "! pause"
		} else {
			buttonPause.text = "pause"
		}
		buttonPause.draw()

		if buttonSI.isClicked() {
			siUnit = !siUnit
		}
		if siUnit {
			buttonSI.text = "! SI"
		} else {
			buttonSI.text = "SI"
		}
		buttonSI.draw()

		grid.draw()
		drawMetric()
		showForces()

		for _, c := range circles {
			c.draw()
		}

		applyAccAtMousePos()

		rl.ClearBackground(rl.Black)
		rl.EndDrawing()
	}
}

func (c *Circle) draw() {
	rl.DrawCircle(int32(c.pos.X), int32(c.pos.Y), float32(c.radius), c.col)
}

func (c *Circle) move() {
	deltaTime := fixedDeltaTime

	// Calculate velocity
	c.vel.X += c.acc.X * deltaTime
	c.vel.Y += c.acc.Y * deltaTime

	// Calculate position
	c.pos.X += c.vel.X * deltaTime
	c.pos.Y += c.vel.Y * deltaTime

	// Handle Collision
	if c.pos.X-c.radius < 0 {
		c.pos.X = 0 + c.radius
		c.vel.X *= -1 * dumpingFactor
	} else if c.pos.X+c.radius > float32(spaceWidth) {
		c.pos.X = float32(spaceWidth) - c.radius
		c.vel.X *= -1 * dumpingFactor
	}

	if c.pos.Y-c.radius < 0 {
		c.pos.Y = 0 + c.radius
		c.vel.Y *= -1 * dumpingFactor
	} else if c.pos.Y+c.radius > float32(spaceHeight) {
		c.pos.Y = float32(spaceHeight) - c.radius
		c.vel.Y *= -1 * dumpingFactor
	}
}

func applyAccAtMousePos() {
	mousePos := rl.GetMousePosition()
	if rl.IsMouseButtonDown(rl.MouseLeftButton) && mouseInSpace() && !pause {
		for i := range circles {
			circles[i].acc = rl.Vector2Scale(rl.Vector2Normalize(rl.Vector2Subtract(mousePos, circles[i].pos)), 700)
		}
	} else {
		if !pause {
			for i := range circles {
				circles[i].acc = rl.NewVector2(0, 700)
			}
		}
	}
}

func mouseInSpace() bool {
	mousePos := rl.GetMousePosition()
	if mousePos.X < float32(spaceWidth) && mousePos.Y < float32(spaceHeight) {
		return true
	}
	return false
}

func drawMetric() {
	var y int32 = 20
	var n int32 = 1
	if metric {
		for _, c := range primaryCircles {
			posX := c.pos.X
			posY := c.pos.Y
			velX := c.vel.X
			velY := c.vel.Y
			accX := c.acc.X
			accY := c.acc.Y
			unit := "p/s"

			if siUnit {
				posX = pixelToSI(posX)
				posY = pixelToSI(posY)
				velX = pixelToSI(velX)
				velY = pixelToSI(velY)
				accX = pixelToSI(accX)
				accY = pixelToSI(accY)
				unit = "m/s"
			}

			// position
			rl.DrawText(fmt.Sprintf("Position %d: (%.2f, %.2f)", n, posX, posY), 20, y, 10, rl.Gray)
			rl.DrawCircle(10, y+5, 5, c.col)

			// velocity
			rl.DrawText(fmt.Sprintf("Velocity %d: (%.2f, %.2f) %s", n, velX, velY, unit), 220, y, 10, rl.Gray)
			rl.DrawCircle(210, y+5, 5, c.col)

			// acceleration
			rl.DrawText(fmt.Sprintf("Acceleration %d: (%.2f, %.2f) %s", n, accX, accY, unit), 420, y, 10, rl.Gray)
			rl.DrawCircle(410, y+5, 5, c.col)

			y += 20
			n += 1
		}

		// mouse
		mousePos := rl.GetMousePosition()
		rl.DrawText(fmt.Sprintf("Mouse: %f, %f", mousePos.X, mousePos.Y), 620, 20, 10, rl.Gray)
	}
}

func showForces() {
	if forces {
		for _, c := range circles {
			arrow := Arrow{
				rl.NewVector2(c.pos.X, c.pos.Y),
				rl.Vector2Add(c.pos, rl.Vector2Scale(rl.Vector2Normalize(c.acc), 100)),
			}
			arrow.draw()
		}
	}
}

var circlesSec []Circle

func playPause() {
	if pause {
		circlesSec = make([]Circle, len(circles))
		copy(circlesSec, circles)
		for i := range circles {
			circles[i].vel = rl.NewVector2(0, 0)
			circles[i].acc = rl.NewVector2(0, 0)
		}
		primaryCircles = circlesSec
	} else {
		for i := range circles {
			circles[i].vel = circlesSec[i].vel
			circles[i].acc = circlesSec[i].acc
		}
		primaryCircles = circles
	}
}

// 1 meter = 20px
func pixelToSI(pixel float32) float32 {
	return pixel / float32(pixelPerMetre)
}

func toPixel(si float32) float32 {
	return si * float32(pixelPerMetre)
}
