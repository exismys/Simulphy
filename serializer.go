package main

import (
	"encoding/json"
	"fmt"
	"os"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type SimState struct {
	AndGates []*AndGate
	OrGates  []*OrGate
	NotGates []*NotGate
	Leds     []*Led
	Wires    []*Wire
	Powers   []*Power
	PortMap  map[int32]*Port
}

func serialize() {
	simState := &SimState{
		AndGates: andGates,
		OrGates:  orGates,
		NotGates: notGates,
		Leds:     leds,
		Wires:    wires,
		Powers:   powers,
		PortMap:  portMap,
	}

	s, error := json.Marshal(simState)
	if error != nil {
		fmt.Println(error)
	}
	os.WriteFile("circuit.sim.json", s, 0644)
}

func deserialize(sim *Simulation) {
	data, err := os.ReadFile("circuit.sim.json")
	if err != nil {
		fmt.Println(err)
		return
	}

	simState := &SimState{}
	err = json.Unmarshal(data, simState)
	if err != nil {
		fmt.Println(err)
		return
	}

	andGates = simState.AndGates
	orGates = simState.OrGates
	notGates = simState.NotGates
	leds = simState.Leds
	wires = simState.Wires
	powers = simState.Powers
	portMap = simState.PortMap

	// Attach functions and populate unserializable simulation fields,
	// and restore references by looking up port ID to port object map
	for _, l := range leds {
		l.InputPort = portMap[l.InputPortId]
		restoreReferences(l.InputPort)
		attachFuncL(sim, l)
		sim.Objects = append(sim.Objects, l)
	}
	for _, ag := range andGates {
		ag.InputPortA = portMap[ag.InputPortAId]
		ag.InputPortB = portMap[ag.InputPortBId]
		ag.OutputPort = portMap[ag.OutputPortId]
		restoreReferences(ag.InputPortA)
		restoreReferences(ag.InputPortB)
		restoreReferences(ag.OutputPort)
		attachFuncAg(sim, ag)
		sim.Objects = append(sim.Objects, ag)
	}
	for _, og := range orGates {
		og.InputPortA = portMap[og.InputPortAId]
		og.InputPortB = portMap[og.InputPortBId]
		og.OutputPort = portMap[og.OutputPortId]
		restoreReferences(og.InputPortA)
		restoreReferences(og.InputPortB)
		restoreReferences(og.OutputPort)
		attachFuncOg(sim, og)
		sim.Objects = append(sim.Objects, og)
	}
	for _, ng := range notGates {
		ng.InputPort = portMap[ng.InputPortId]
		ng.OutputPort = portMap[ng.OutputPortId]
		restoreReferences(ng.InputPort)
		restoreReferences(ng.OutputPort)
		attachFuncNg(sim, ng)
		sim.Objects = append(sim.Objects, ng)
	}
	for _, p := range powers {
		p.OutputPort = portMap[p.OutputPortId]
		restoreReferences(p.OutputPort)
		attachFuncP(sim, p)
		sim.Objects = append(sim.Objects, p)
	}
	for _, w := range wires {
		w.FromPort = portMap[w.FromPort.Id]
		w.ToPort = portMap[w.ToPort.Id]
		restoreReferences(w.FromPort)
		restoreReferences(w.ToPort)
		sim.Objects = append(sim.Objects, w)
	}
}

func restoreReferences(port *Port) {
	if port.IsInputPort {
		for _, p := range port.FromPortsIds {
			port.FromPorts = append(port.FromPorts, portMap[p])
		}
	} else {
		for _, p := range port.InputPortsIds {
			port.InputPorts = append(port.InputPorts, portMap[p])
		}
	}
}

func attachFuncP(sim *Simulation, p *Power) {
	p.OutputPort.onClick = func() {
		fmt.Println("Output port of POWER SOURCE clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(p.OutputPort.Pos, sim.CameraOffset),
			FromPort: p.OutputPort,
		}
		sim.GhostObject = &wire
	}
}

func attachFuncL(sim *Simulation, l *Led) {
	l.InputPort.onClick = func() {
		fmt.Println("Input port of the Led clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = l.InputPort.Pos
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			sim.Objects = append(sim.Objects, w)
			wires = append(wires, w)
			l.InputPort.FromPorts = append(l.InputPort.FromPorts, w.FromPort)
			finalPort = l.InputPort
			l.State = calculateState(finalPort)
			fmt.Println("Led State: ", l.State)
			fmt.Println("Number of wires: ", len(wires))
			sim.GhostObject = nil
		}
	}
}

func attachFuncAg(sim *Simulation, ag *AndGate) {
	ag.InputPortA.onClick = func() {
		fmt.Println("Input port of AND gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = ag.InputPortA.Pos
			w.ToPort = ag.InputPortA
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			ag.InputPortA.FromPorts = append(ag.InputPortA.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	ag.InputPortB.onClick = func() {
		fmt.Println("Input port of AND gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = ag.InputPortB.Pos
			w.ToPort = ag.InputPortB
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			ag.InputPortB.FromPorts = append(ag.InputPortB.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	ag.OutputPort.onClick = func() {
		fmt.Println("Output port of AND gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(ag.OutputPort.Pos, sim.CameraOffset),
			FromPort: ag.OutputPort,
		}
		sim.GhostObject = &wire
	}
}

func attachFuncOg(sim *Simulation, og *OrGate) {
	og.InputPortA.onClick = func() {
		fmt.Println("Input port of OR gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = og.InputPortA.Pos
			w.ToPort = og.InputPortA
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			og.InputPortA.FromPorts = append(og.InputPortA.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	og.InputPortB.onClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = og.InputPortB.Pos
			w.ToPort = og.InputPortB
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			og.InputPortB.FromPorts = append(og.InputPortB.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	og.OutputPort.onClick = func() {
		fmt.Println("Output port of OR gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(og.OutputPort.Pos, sim.CameraOffset),
			FromPort: og.OutputPort,
		}
		sim.GhostObject = &wire
	}
}

func attachFuncNg(sim *Simulation, ng *NotGate) {
	ng.InputPort.onClick = func() {
		fmt.Println("Input port of NOT gate clicked!")
		if sim.GhostObject != nil {
			w := sim.GhostObject.(*Wire)
			w.To = ng.InputPort.Pos
			w.ToPort = ng.InputPort
			w.From = rl.Vector2Add(w.From, sim.CameraOffset)
			wires = append(wires, w)
			ng.InputPort.FromPorts = append(ng.InputPort.FromPorts, w.FromPort)
			fmt.Println("Number of wires: ", len(wires))
			sim.Objects = append(sim.Objects, w)
			sim.GhostObject = nil
		}
	}
	ng.OutputPort.onClick = func() {
		fmt.Println("Output port of NOT gate clicked")
		wire := Wire{
			From:     rl.Vector2Subtract(ng.OutputPort.Pos, sim.CameraOffset),
			FromPort: ng.OutputPort,
		}
		sim.GhostObject = &wire
	}
}
