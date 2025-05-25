package main

import "fmt"

var wires []*Wire
var ports []*Port

var finalPort *Port

func calculateState(p *Port) bool {
	if p.inputPort {
		fmt.Println("Resolving state for the current input port")
		for _, fp := range p.fromPorts {
			p.state = p.state || calculateState(fp)
		}
		fmt.Println("Returning state: ", p.state)
		return p.state
	} else {
		if p.resMethod == NOT {
			fmt.Println("Resolving state with method: ", p.resMethod)
			for _, ip := range p.inputPorts {
				p.state = !calculateState(ip)
			}
		} else if p.resMethod == OR {
			for _, ip := range p.inputPorts {
				p.state = p.state || calculateState(ip)
			}
		} else if p.resMethod == AND {
			p.state = true
			for _, ip := range p.inputPorts {
				p.state = p.state && calculateState(ip)
			}
		} else {
			fmt.Println("Not a valid resMethod", p.resMethod)
		}
		return p.state
	}
}
