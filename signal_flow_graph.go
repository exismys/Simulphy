package main

import "fmt"

var wires []*Wire
var ports []*Port
var leds []*Led

var finalPort *Port

func calculateState(p *Port) bool {
	if p.inputPort {
		state := p.state
		for _, fp := range p.fromPorts {
			state = state || calculateState(fp)
		}
		fmt.Println("Returning state: ", state)
		return state
	}
	state := p.state
	if p.resMethod == NOT {
		for _, ip := range p.inputPorts {
			state = !calculateState(ip)
		}
	} else if p.resMethod == OR {
		state = false
		for _, ip := range p.inputPorts {
			state = state || calculateState(ip)
		}
	} else if p.resMethod == AND {
		state = true
		for _, ip := range p.inputPorts {
			state = state && calculateState(ip)
		}
	} else if p.resMethod == NONE {
	}
	fmt.Println("Returning state: ", state)
	return state
}

func refreshState() {
	for _, led := range leds {
		led.state = calculateState(led.inputPort)
	}
}
