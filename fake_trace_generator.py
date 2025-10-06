#!/usr/bin/env python3
import argparse
import json
import random
import socket
import string
import sys
import threading
import time
from datetime import datetime, timedelta, timezone

# --------------------------
# CONFIGURATION PARAMETERS
# --------------------------

ISSUERS = [
    "US Treasury", "IBM", "Apple", "Microsoft",
    "Johnson & Johnson", "Fannie Mae", "Goldman Sachs",
    "Citi", "Amazon", "Pfizer"
]

ON_TIME_THRESHOLD_SECONDS = 15 * 60  # 15 minutes

# --------------------------
# CUSIP GENERATION
# --------------------------

def cusip_check_digit(cusip_base):
    """Compute the official CUSIP check digit."""
    def cusip_value(c):
        if c.isdigit():
            return int(c)
        elif c.isalpha():
            return ord(c.upper()) - 55
        elif c == '*':
            return 36
        elif c == '@':
            return 37
        elif c == '#':
            return 38
        else:
            raise ValueError(f"Invalid CUSIP character: {c}")

    values = [cusip_value(c) for c in cusip_base]
    total = 0
    for i, v in enumerate(values):
        if i % 2 == 1:
            v *= 2
        total += v // 10 + v % 10
    check = (10 - (total % 10)) % 10
    return str(check)


def generate_random_cusip():
    """Generate a valid 9-character CUSIP."""
    base = ''.join(random.choices(string.ascii_uppercase + string.digits, k=8))
    check = cusip_check_digit(base)
    return base + check


def random_issuer():
    return random.choice(ISSUERS)


def random_price():
    return round(random.uniform(90.0, 110.0), 3)


def random_volume():
    return random.randint(100000, 5000000)


# --------------------------
# TRADE GENERATION
# --------------------------

def make_trade(cusip=None, exec_time=None, pair_id=None, side=None, dealer_id=None):
    """Generate one trade leg."""
    cusip = cusip or generate_random_cusip()
    issuer = random_issuer()
    now = datetime.now(timezone.utc)  # timezone-aware UTC

    exec_time = exec_time or (now - timedelta(seconds=random.randint(0, 600)))
    report_delay = random.randint(0, 1800)
    report_time = exec_time + timedelta(seconds=report_delay)
    late = (report_time - exec_time).total_seconds() > ON_TIME_THRESHOLD_SECONDS
    modifier3 = "Z" if late else ""

    msg = {
        "control_id": pair_id or ''.join(random.choices(string.ascii_uppercase + string.digits, k=10)),
        "cusip": cusip,
        "issuer": issuer,
        "exec_time": exec_time.isoformat().replace("+00:00", "Z"),
        "report_time": report_time.isoformat().replace("+00:00", "Z"),
        "price": random_price(),
        "volume": random_volume(),
        "side": side or random.choice(["BUY", "SELL"]),
        "dealer_id": dealer_id or random.randint(1000, 9999),
        "reporting_capacity": random.choice(["P", "A"]),
        "modifier3": modifier3,
        "coupon": round(random.uniform(1.0, 6.0), 2),
        "maturity": (now + timedelta(days=random.randint(365, 3650))).date().isoformat()
    }
    return msg


# --------------------------
# TCP SOCKET THREAD (with disconnect handling)
# --------------------------

def socket_server_thread(host, port, queue):
    """Continuously send messages in the queue to connected clients."""
    print(f"[TRACE FEED] Starting TCP server on {host}:{port}", file=sys.stderr)
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen(1)

        while True:
            print(f"[TRACE FEED] Waiting for client connection...", file=sys.stderr)
            try:
                conn, addr = s.accept()
                print(f"[TRACE FEED] Client connected from {addr}", file=sys.stderr)
            except Exception as e:
                print(f"[TRACE FEED] Accept failed: {e}", file=sys.stderr)
                continue

            with conn:
                while True:
                    if queue:
                        msg = queue.pop(0)
                        try:
                            conn.sendall((json.dumps(msg) + "\n").encode())
                        except (BrokenPipeError, ConnectionResetError):
                            print(f"[TRACE FEED] Client disconnected", file=sys.stderr)
                            break  # Exit inner loop to accept new connection
                        except Exception as e:
                            print(f"[TRACE FEED] Unexpected error sending message: {e}", file=sys.stderr)
                            break
                    else:
                        time.sleep(0.001)


# --------------------------
# MAIN LOOP
# --------------------------

def main():
    parser = argparse.ArgumentParser(description="Fake TRACE feed generator")
    parser.add_argument("--rate", type=float, default=1.0, help="messages per second")
    parser.add_argument("--rate-jitter", type=float, default=0.0, help="fractional jitter to apply to sleep interval (0.0-1.0)")
    parser.add_argument("--tcp", action="store_true", help="enable TCP feed output")
    parser.add_argument("--pairs", action="store_true", help="emit both legs of some trades")
    parser.add_argument("--pair-prob", type=float, default=0.3, help="probability of emitting a paired trade (0.0 to 1.0)")
    parser.add_argument("--host", default="127.0.0.1", help="TCP host")
    parser.add_argument("--port", type=int, default=5555, help="TCP port")
    parser.add_argument("--burst", type=int, default=0, help="number of messages in each burst")
    parser.add_argument("--burst-interval", type=int, default=60, help="seconds between bursts")
    parser.add_argument("--out-file", type=str, help="optional file to write all messages")
    args = parser.parse_args()

    msg_queue = []
    last_burst_time = time.time()

    if args.tcp:
        threading.Thread(
            target=socket_server_thread, args=(args.host, args.port, msg_queue), daemon=True
        ).start()

    print(f"[TRACE FEED] Generating {args.rate} msg/sec | TCP={'on' if args.tcp else 'off'} | "
          f"Pairs={'on' if args.pairs else 'off'} | Burst={args.burst} every {args.burst_interval}s | "
          f"PairProb={args.pair_prob} | RateJitter={args.rate_jitter} | OutFile={'on' if args.out_file else 'off'}",
          file=sys.stderr)

    try:
        while True:
            now_time = time.time()

            # ------------------------
            # Handle periodic bursts
            # ------------------------
            if args.burst > 0 and (now_time - last_burst_time) >= args.burst_interval:
                print(f"[TRACE FEED] Emitting burst of {args.burst} messages...", file=sys.stderr)
                for _ in range(args.burst):
                    msg = make_trade()
                    msg_str = json.dumps(msg)
                    print(msg_str, flush=True)
                    if args.tcp:
                        msg_queue.append(msg)
                    if args.out_file:
                        with open(args.out_file, "a") as f:
                            f.write(msg_str + "\n")

                    if args.pairs and random.random() < args.pair_prob:
                        pair_id = msg["control_id"]
                        paired_msg = make_trade(
                            cusip=msg["cusip"],
                            exec_time=datetime.fromisoformat(msg["exec_time"].replace("Z", "")),
                            pair_id=pair_id,
                            side="BUY" if msg["side"] == "SELL" else "SELL",
                            dealer_id=random.randint(1000, 9999),
                        )
                        paired_str = json.dumps(paired_msg)
                        print(paired_str, flush=True)
                        if args.tcp:
                            msg_queue.append(paired_msg)
                        if args.out_file:
                            with open(args.out_file, "a") as f:
                                f.write(paired_str + "\n")
                last_burst_time = now_time
                continue

            # ------------------------
            # Normal rate-controlled message
            # ------------------------
            msg = make_trade()
            msg_str = json.dumps(msg)
            print(msg_str, flush=True)
            if args.tcp:
                msg_queue.append(msg)
            if args.out_file:
                with open(args.out_file, "a") as f:
                    f.write(msg_str + "\n")

            if args.pairs and random.random() < args.pair_prob:
                pair_id = msg["control_id"]
                paired_msg = make_trade(
                    cusip=msg["cusip"],
                    exec_time=datetime.fromisoformat(msg["exec_time"].replace("Z", "")),
                    pair_id=pair_id,
                    side="BUY" if msg["side"] == "SELL" else "SELL",
                    dealer_id=random.randint(1000, 9999),
                )
                paired_str = json.dumps(paired_msg)
                time.sleep(random.uniform(0.1, 0.5))
                print(paired_str, flush=True)
                if args.tcp:
                    msg_queue.append(paired_msg)
                if args.out_file:
                    with open(args.out_file, "a") as f:
                        f.write(paired_str + "\n")

            # ------------------------
            # Sleep with optional jitter
            # ------------------------
            interval = 1.0 / args.rate
            if args.rate_jitter > 0.0:
                jitter = random.uniform(-interval * args.rate_jitter, interval * args.rate_jitter)
                interval += jitter
            time.sleep(max(0.001, interval))

            # Prevent unbounded queue growth
            if args.tcp and len(msg_queue) > 2000:
                msg_queue = msg_queue[-2000:]

    except KeyboardInterrupt:
        print("\n[TRACE FEED] Stopped.", file=sys.stderr)


if __name__ == "__main__":
    main()
