package main

import (
	"bufio"
	"cooldb/utils"
	"flag"
	"fmt"
	"os"
	"strings"
	"time"
)

func measureTime(f func() error) (time.Duration, error) {
	start := time.Now()
	err := f()
	duration := time.Since(start)
	return duration, err
}

func main() {
	dbPath := flag.String("a", "cooldb.cdb", "Database file path")
	flag.Parse()

	db, err := utils.NewDB(*dbPath)
	if err != nil {
		fmt.Printf("Error opening database: %v\n", err)
		os.Exit(1)
	}
	defer db.Close()

	reader := bufio.NewReader(os.Stdin)
	fmt.Println("CoolDB Interactive Terminal")
	fmt.Println("Available commands:")
	fmt.Println("  begin                   - Start a new transaction")
	fmt.Println("  set <key> <value>       - Set a key-value pair")
	fmt.Println("  get <key>               - Retrieve a value")
	fmt.Println("  delete <key>            - Delete a key")
	fmt.Println("  commit <txid>           - Commit a transaction")
	fmt.Println("  rollback <txid>         - Rollback a transaction")
	fmt.Println("  exit                    - Exit the terminal")

	var currentTxID int64 = 0

	for {
		fmt.Print("cooldb> ")
		input, _ := reader.ReadString('\n')
		input = strings.TrimSpace(input)
		parts := strings.Fields(input)

		if len(parts) == 0 {
			continue
		}

		switch parts[0] {
		case "begin":
			currentTxID = db.Begin()
			fmt.Printf("Started transaction %d\n", currentTxID)

		case "set":
			if len(parts) < 3 {
				fmt.Println("Usage: set <key> <value>")
				continue
			}
			if currentTxID == 0 {
				fmt.Println("No active transaction. Use 'begin' first.")
				continue
			}
			duration, err := measureTime(func() error {
				return db.Set(currentTxID, parts[1], parts[2])
			})
			if err != nil {
				fmt.Printf("Error: %v\n", err)
			} else {
				fmt.Printf("Set %s = %s in transaction %d (took %v ms)\n",
					parts[1], parts[2], currentTxID, duration.Milliseconds())
			}

		case "get":
			if len(parts) < 2 {
				fmt.Println("Usage: get <key>")
				continue
			}
			start := time.Now()
			value, exists := db.Get(parts[1])
			duration := time.Since(start)
			if exists {
				fmt.Printf("%s = %s (took %v ms)\n", parts[1], value, duration.Milliseconds())
			} else {
				fmt.Printf("Key %s not found (took %v ms)\n", parts[1], duration.Milliseconds())
			}

		case "delete":
			if len(parts) < 2 {
				fmt.Println("Usage: delete <key>")
				continue
			}
			if currentTxID == 0 {
				fmt.Println("No active transaction. Use 'begin' first.")
				continue
			}
			duration, err := measureTime(func() error {
				return db.Delete(currentTxID, parts[1])
			})
			if err != nil {
				fmt.Printf("Error: %v\n", err)
			} else {
				fmt.Printf("Deleted %s in transaction %d (took %v ms)\n",
					parts[1], currentTxID, duration.Milliseconds())
			}

		case "commit":
			if currentTxID == 0 {
				fmt.Println("No active transaction to commit")
				continue
			}
			txID := currentTxID
			duration, err := measureTime(func() error {
				return db.Commit(txID)
			})
			if err != nil {
				fmt.Printf("Commit error: %v\n", err)
			} else {
				fmt.Printf("Transaction %d committed (took %v ms)\n", txID, duration.Milliseconds())
				currentTxID = 0
			}

		case "rollback":
			if currentTxID == 0 {
				fmt.Println("No active transaction to rollback")
				continue
			}
			txID := currentTxID
			duration, err := measureTime(func() error {
				return db.Rollback(txID)
			})
			if err != nil {
				fmt.Printf("Rollback error: %v\n", err)
			} else {
				fmt.Printf("Transaction %d rolled back (took %v ms)\n", txID, duration.Milliseconds())
				currentTxID = 0
			}

		case "exit":
			fmt.Println("Exiting CoolDB...")
			return

		default:
			fmt.Println("Unknown command. Try again.")
		}
	}
}
