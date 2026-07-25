#!/usr/bin/env python3
"""PC joystick verisini Deneyap Kart'a Wi-Fi/UDP ile gonderir."""

from __future__ import annotations

import argparse
import socket
import sys
import time

import pygame


def clamp(value: int, minimum: int, maximum: int) -> int:
    return max(minimum, min(maximum, value))


def axis_percent(value: float, invert: bool = False) -> int:
    if invert:
        value = -value
    return clamp(round(value * 100), -100, 100)


def throttle_percent(value: float) -> int:
    # Pygame ekseni genellikle -1 konumunda tam ileri, +1 konumunda tam geri gelir.
    return clamp(round((1.0 - value) * 50.0), 0, 100)


def hat_angle(hat: tuple[int, int]) -> int:
    mapping = {
        (0, 0): -1,
        (0, 1): 0,
        (1, 1): 45,
        (1, 0): 90,
        (1, -1): 135,
        (0, -1): 180,
        (-1, -1): 225,
        (-1, 0): 270,
        (-1, 1): 315,
    }
    return mapping.get(hat, -1)


def button_mask(joystick: pygame.joystick.Joystick) -> int:
    mask = 0
    for index in range(min(joystick.get_numbuttons(), 32)):
        if joystick.get_button(index):
            mask |= 1 << index
    return mask


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="PC joystick verisini METEBOT'a Wi-Fi/UDP ile gonderir."
    )
    parser.add_argument("--robot-ip", default="192.168.4.1")
    parser.add_argument("--port", type=int, default=4210)
    parser.add_argument("--rate", type=float, default=50.0)
    parser.add_argument("--joystick-index", type=int, default=0)
    parser.add_argument("--x-axis", type=int, default=0)
    parser.add_argument("--y-axis", type=int, default=1)
    parser.add_argument("--twist-axis", type=int, default=2)
    parser.add_argument("--throttle-axis", type=int, default=3)
    parser.add_argument("--show", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.rate <= 0:
        raise ValueError("--rate sifirdan buyuk olmali")

    pygame.init()
    pygame.joystick.init()

    if pygame.joystick.get_count() <= args.joystick_index:
        print("Joystick bulunamadi.", file=sys.stderr)
        return 1

    joystick = pygame.joystick.Joystick(args.joystick_index)
    joystick.init()

    required_axis = max(
        args.x_axis,
        args.y_axis,
        args.twist_axis,
        args.throttle_axis,
    )
    if joystick.get_numaxes() <= required_axis:
        print(
            f"Joystickte yeterli eksen yok. Bulunan: {joystick.get_numaxes()}",
            file=sys.stderr,
        )
        return 1

    destination = (args.robot_ip, args.port)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    interval = 1.0 / args.rate

    print(f"Joystick: {joystick.get_name()}")
    print(f"Hedef: {args.robot_ip}:{args.port}")
    print("Cikmak icin Ctrl+C")

    try:
        while True:
            started = time.monotonic()
            pygame.event.pump()

            x = axis_percent(joystick.get_axis(args.x_axis))
            y = axis_percent(joystick.get_axis(args.y_axis), invert=True)
            twist = axis_percent(joystick.get_axis(args.twist_axis))
            throttle = throttle_percent(joystick.get_axis(args.throttle_axis))
            hat = hat_angle(joystick.get_hat(0)) if joystick.get_numhats() else -1
            buttons = button_mask(joystick)

            packet = f"JOY,{x},{y},{twist},{throttle},{hat},{buttons}\n"
            sock.sendto(packet.encode("ascii"), destination)

            if args.show:
                print(packet.strip(), end="\r", flush=True)

            elapsed = time.monotonic() - started
            time.sleep(max(0.0, interval - elapsed))

    except KeyboardInterrupt:
        # Program kapanirken robotun son hareket komutunda kalmamasina yardimci olur.
        neutral = "JOY,0,0,0,0,-1,0\n".encode("ascii")
        for _ in range(3):
            sock.sendto(neutral, destination)
            time.sleep(0.02)
        print("\nDurduruldu.")
        return 0
    finally:
        sock.close()
        pygame.quit()


if __name__ == "__main__":
    raise SystemExit(main())
