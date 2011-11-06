/*
 * Protocol:
 *
 * SERVER
 * ======
 *
 * Connecting messages:
 *
 * CONNECT
 * - OK
 * AUTH <username> <password>
 * - OK
 * - FAIL: INCORRECT LOGIN
 * DISCONNECT
 *
 * Project messages:
 *
 * PROJECT LIST
 * - list of (name, id)
 *
 * PROJECT OPEN <id> [<password>]
 * - OK (stream data)
 *
 * PROJECT CLOSE
 * - OK
 * - FAIL: UNKNOWN ERROR
