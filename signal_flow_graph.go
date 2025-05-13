package main

var wires []*Wire

var root *Port

func GetConnectionGraph() {
	for _, w := range wires {
		if w.FromPort == nil {
			root = w.FromPort
		}

	}
}
