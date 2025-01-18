package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func MakeDB(filename string) {
	// Open the file with O_CREATE and O_EXCL flags
	file, err := os.OpenFile(filename, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0644)
	if err != nil {
		if os.IsExist(err) {
			fmt.Printf("File %s already exists, skipping creation.\n", filename)
		} else {
			fmt.Printf("Failed to create file: %v\n", err)
		}
		return
	}
	defer file.Close()

	fmt.Printf("File %s created successfully.\n", filename)
}

func Set(filename string, key string, value string) {
	// Open the file in append mode, create it if it doesn't exist
	file, err := os.OpenFile(filename, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		fmt.Printf("Failed to open file: %v\n", err)
		return
	}
	defer file.Close()

	// Data to append
	data := key + ":" + value + "\n"

	// Write data to the file
	_, err = file.WriteString(data)
	if err != nil {
		fmt.Printf("Failed to write to file: %v\n", err)
		return
	}

	fmt.Printf("Data appended successfully to %s\n", filename)
}

func countLines(filename string) (int, []string, error) {
	// Open the file
	file, err := os.Open(filename)
	if err != nil {
		return 0, nil, fmt.Errorf("failed to open file: %w", err)
	}
	defer file.Close()

	// Create a scanner to read the file line by line
	scanner := bufio.NewScanner(file)
	var lines []string
	lineCount := 0

	// Loop through each line
	for scanner.Scan() {
		lineCount++
		lines = append(lines, scanner.Text())
	}

	// Check for errors during scanning
	if err := scanner.Err(); err != nil {
		return 0, nil, fmt.Errorf("error reading file: %w", err)
	}

	return lineCount, lines, nil
}

func splitKeyValue(data string) (string, string, error) {
	parts := strings.Split(data, ":")
	if len(parts) != 2 {
		return "", "", fmt.Errorf("invalid format, expected 'key:value'")
	}
	return parts[0], parts[1], nil
}

func Get(filename string, key string) {
	_, lines, err := countLines(filename)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		return
	}


	for _, line := range lines {
		k, v, err := splitKeyValue(line)
		if err != nil {
			fmt.Printf("Skipping invalid line: %s\n", line)
			continue
		}

		if k == key {
			fmt.Printf("Value found for key '%s': %s\n", key, v)
			return
		}
	}

	fmt.Printf("Key '%s' not found.\n", key)
}

func main() {
	MakeDB("keyDB.cdb")
	Set("keyDB.cdb", "Rishabh", "Cools9")
	Set("keyDB.cdb", "Rishab", "Cools8")

	Get("keyDB.cdb", "Rishabh")
	Get("keyDB.cdb", "Rishab")
	Get("keyDB.cdb", "UnknownKey")
}
