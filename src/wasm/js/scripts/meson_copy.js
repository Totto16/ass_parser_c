#!/usr/bin/env node

const fs = require('node:fs')


const path = require('node:path')


const raw_source = process.argv[2]

const _raw_dest = process.argv[3]

const source = path.join(process.cwd(), raw_source)

const dest = path.join(__dirname, '..', 'static', 'build', path.basename(raw_source))


console.log(`copying '${source}' to '${dest}'`)

const destDir = path.dirname(dest)

if (!fs.existsSync(destDir)) {
    fs.mkdirSync(destDir, { recursive: true })
}

fs.copyFileSync(source, dest)

