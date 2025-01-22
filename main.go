package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"sync"
)

type Operation struct {
	Type  string `json:"type"`
	Key   string `json:"key"`
	Value string `json:"value"`
	TxID  int64  `json:"txid"`
}

type DB struct {
	filename  string
	cache     map[string]string
	mu        sync.RWMutex
	writer    *bufio.Writer
	file      *os.File
	walFile   *os.File
	walWriter *bufio.Writer
	txCounter int64
	activeTx  map[int64][]Operation
	txMu      sync.Mutex
}

func NewDB(filename string) (*DB, error) {
	dir := filepath.Dir(filename)
	if err := os.MkdirAll(dir, 0755); err != nil {
		return nil, err
	}

	walPath := filename + ".wal"
	walFile, err := os.OpenFile(walPath, os.O_CREATE|os.O_RDWR|os.O_APPEND, 0644)
	if err != nil {
		return nil, err
	}

	file, err := os.OpenFile(filename, os.O_CREATE|os.O_RDWR, 0644)
	if err != nil {
		walFile.Close()
		return nil, err
	}

	db := &DB{
		filename:  filename,
		cache:     make(map[string]string),
		file:      file,
		writer:    bufio.NewWriter(file),
		walFile:   walFile,
		walWriter: bufio.NewWriter(walFile),
		activeTx:  make(map[int64][]Operation),
	}

	if err := db.recover(); err != nil {
		return nil, err
	}

	return db, nil
}

func (db *DB) recover() error {
	scanner := bufio.NewScanner(db.walFile)
	for scanner.Scan() {
		var op Operation
		if err := json.Unmarshal(scanner.Bytes(), &op); err != nil {
			continue
		}
		if op.Type == "commit" {
			db.applyOperation(op)
		}
	}
	return scanner.Err()
}

func (db *DB) Begin() int64 {
	db.txMu.Lock()
	defer db.txMu.Unlock()

	db.txCounter++
	db.activeTx[db.txCounter] = make([]Operation, 0)
	return db.txCounter
}

func (db *DB) Commit(txID int64) error {
	db.txMu.Lock()
	defer db.txMu.Unlock()

	ops, exists := db.activeTx[txID]
	if !exists {
		return fmt.Errorf("transaction %d not found", txID)
	}

	for _, op := range ops {
		data, _ := json.Marshal(op)
		if _, err := db.walWriter.Write(append(data, '\n')); err != nil {
			return err
		}
		db.applyOperation(op)
	}

	if err := db.walWriter.Flush(); err != nil {
		return err
	}

	delete(db.activeTx, txID)
	return nil
}

func (db *DB) Rollback(txID int64) error {
	db.txMu.Lock()
	defer db.txMu.Unlock()

	delete(db.activeTx, txID)
	return nil
}

func (db *DB) Set(txID int64, key, value string) error {
	op := Operation{
		Type:  "set",
		Key:   key,
		Value: value,
		TxID:  txID,
	}

	db.txMu.Lock()
	db.activeTx[txID] = append(db.activeTx[txID], op)
	db.txMu.Unlock()

	return nil
}

func (db *DB) Delete(txID int64, key string) error {
	op := Operation{
		Type: "delete",
		Key:  key,
		TxID: txID,
	}

	db.txMu.Lock()
	db.activeTx[txID] = append(db.activeTx[txID], op)
	db.txMu.Unlock()

	return nil
}

func (db *DB) Get(key string) (string, bool) {
	db.mu.RLock()
	defer db.mu.RUnlock()

	value, exists := db.cache[key]
	return value, exists
}

func (db *DB) applyOperation(op Operation) {
	db.mu.Lock()
	defer db.mu.Unlock()

	switch op.Type {
	case "set":
		db.cache[op.Key] = op.Value
	case "delete":
		delete(db.cache, op.Key)
	}
}

func (db *DB) Close() error {
	db.mu.Lock()
	defer db.mu.Unlock()

	if err := db.walWriter.Flush(); err != nil {
		return err
	}
	if err := db.writer.Flush(); err != nil {
		return err
	}
	db.walFile.Close()
	return db.file.Close()
}

func main() {
	db, err := NewDB("keyDB.cdb")
	if err != nil {
		fmt.Printf("Error creating DB: %v\n", err)
		return
	}
	defer db.Close()

	// Start transaction
	txID := db.Begin()

	// Set operations
	db.Set(txID, "key1", "value1")
	db.Set(txID, "key2", "value2")

	// Delete operation
	db.Delete(txID, "key1")

	// Commit transaction
	if err := db.Commit(txID); err != nil {
		fmt.Printf("Commit error: %v\n", err)
		return
	}

	// Read operations
	if val, exists := db.Get("key2"); exists {
		fmt.Printf("key2: %s\n", val)
	}
	if _, exists := db.Get("key1"); !exists {
		fmt.Println("key1 was deleted")
	}
}
