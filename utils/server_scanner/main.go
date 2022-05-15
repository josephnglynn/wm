package main

import (
	"encoding/binary"
	"log"
	"net/url"
	"os"
	"os/signal"
	"strconv"
	"sync"

	"github.com/gorilla/websocket"
)

type MSG struct {
	t int
}

var wg sync.WaitGroup
var ipAddresses []url.URL

func check(url url.URL) {
	defer wg.Done()
	c, _, err := websocket.DefaultDialer.Dial(url.String(), nil)
	if err != nil {
		return
	}

	defer c.Close()

	done := make(chan struct{})

	interrupt := make(chan os.Signal, 1)
	signal.Notify(interrupt, os.Interrupt)

	go func() {
		defer close(done)
		for {
			_, message, err := c.ReadMessage()
			if err != nil {
				log.Println("read:", err)
				return
			}

			good := true

			// int32 has 4 bytes
			if len(message) > 4 {
				for i := 0; i < 4; i++ {
					if message[i] != 0 {
						good = false
						break
					}
				}

				if good {
					ipAddresses = append(ipAddresses, url)
				}
			}

			return
		}

	}()

	b := make([]byte, 8)
	binary.LittleEndian.PutUint32(b, 0)
	err = c.WriteMessage(websocket.BinaryMessage, b)

	if err != nil {
		return
	}

	for {
		select {
		case <-done:
			return
		case <-interrupt:
			log.Println("interrupt")

			err := c.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseNormalClosure, ""))
			if err != nil {
				log.Println("write close:", err)
				return
			}
			select {
			case <-done:
			}
			return
		}
	}
}

func main() {
	for b := 0; b < 256; b++ {
		for c := 0; c < 256; c++ {
			wg.Add(1)
			host := "192.168." + strconv.Itoa(b) + "." + strconv.Itoa(c) + ":16812"
			u := url.URL{Scheme: "ws", Host: host, Path: "/ws"}
			log.Printf("connecting to %s", u.String())
			go check(u)
		}
	}

	wg.Wait()

	log.Println("TOTAL NUM OF SUCCESSFUL CONECTIONS: ", len(ipAddresses))

	for i := 0; i < len(ipAddresses); i++ {
		log.Println(ipAddresses[i])
	}
}
