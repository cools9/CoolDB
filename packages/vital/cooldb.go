package cooldb
import (
	"fmt"
)

var keys []string
var values []string

func Set(key string, value string) {
	keys = append(keys, key)
	values = append(values, value)
}

func Get(key string) (string, bool) {
	for i, k := range keys {
		if k == key {
			return values[i], true
		}
	}
	return "", false
}

func List() {
	fmt.Println("Keys:")
	for _, key := range keys {
		fmt.Println(key)
	}
}

func Status(){
	fmt.Println(
		"OK"
	)
}

/*
func main() {
	// Measure time taken to set a value
	startSet := time.Now()
	set("hello", "world")
	elapsedSet := time.Since(startSet)
	fmt.Printf("Time taken to set value: %v\n", elapsedSet)

	list()

	// Measure time taken to get a value
	startGet := time.Now()
	value, found := get("hello")
	elapsedGet := time.Since(startGet)

	if found {
		fmt.Printf("Value retrieved: %s\n", value)
	} else {
		fmt.Println("Key not found")
	}

	fmt.Printf("Time taken to get value: %v\n", elapsedGet)
}
*/
