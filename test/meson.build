crunchpp = find_program('crunch++', version: ['>= 1.0.1', '< 2.0.0'])
libcrunchpp = dependency('crunch++', version: ['>= 1.0.1', '< 2.0.0'])

commandExtra = ['-I../substrate', '-std=' + get_option('cpp_std')]
if get_option('b_coverage')
	commandExtra += ['--coverage']
endif
if get_option('debug')
	commandExtra += ['--debug']
endif
if cxx.get_id() != 'msvc'
	commandExtra += ['-z', 'defs']
endif
if get_option('b_lto')
	commandExtra += ['-flto']
endif

if sanitizer != 'none'
	commandExtra += ['-fsanitize=' + sanitizer]
endif

pcatTests = [
	'testFD', 'testConsole', 'testArgsTokenizer', 'testArgsParser',
	'testThreadedQueue', 'testAffinity', 'testThreadPool', 'testMappingOffset',
	'testMMap', 'testIndexSequence', 'testPcat'
]

if host_machine.system() != 'windows'
	testPTY = ['../substrate/impl/pty.cxx']
else
	testPTY = []
endif

testHelpers = static_library(
	'testHelpers',
	[
		'fd.cxx', 'console.cxx', testPTY, 'tokenizer.cxx',
		'argsParser.cxx', 'threadedQueue.cxx', '@0@/affinity.cxx'.format(host_machine.system()), 'threadPool.cxx',
		'mappingOffset.cxx', 'mmap.cxx', 'indexSequence.cxx', 'version.cxx', versionHeader
	],
	pic: true,
	dependencies: [libcrunchpp],
	include_directories: [include_directories('../src'), substrate, platformHeaders],
	install: false,
	build_by_default: true
)

testObjectMap = {
	'testFD': {'test': ['fd.cxx']},
	'testConsole': {
		'test': ['console.cxx', testPTY],
		'pcat': ['substrate/impl/console.cxx']
	},
	'testArgsTokenizer': {'test': ['tokenizer.cxx'], 'pcat': ['src/args/tokenizer.cxx']},
	'testArgsParser': {
		'test': ['argsParser.cxx'],
		'pcat': [
			'src/args.cxx', 'src/args/tokenizer.cxx', 'src/args/types.cxx',
			'substrate/impl/console.cxx'
		]
	},
	'testThreadedQueue': {'test': ['threadedQueue.cxx']},
	'testAffinity': {
		'test': ['@0@/affinity.cxx'.format(host_machine.system())],
		'pcat': [
			'src/args.cxx', 'src/args/tokenizer.cxx', 'src/args/types.cxx',
			'substrate/impl/console.cxx'
		]
	},
	'testThreadPool' : {
		'test': ['threadPool.cxx'],
		'pcat': [
			'src/args.cxx', 'src/args/tokenizer.cxx', 'src/args/types.cxx',
			'substrate/impl/console.cxx'
		]
	},
	'testMappingOffset' : {'test': ['mappingOffset.cxx']},
	'testMMap' : {'test': ['mmap.cxx']},
	'testIndexSequence': {'test': ['indexSequence.cxx']},
	'testPcat': {
		'test': ['version.cxx'],
		'pcat': ['substrate/impl/console.cxx']
	}
}

foreach test : pcatTests
	map = testObjectMap.get(test, {})
	pcatObjs = map.has_key('pcat') ? [pcat.extract_objects(map['pcat'])] : []
	testObjs = map.has_key('test') ? [testHelpers.extract_objects(map['test'])] : []
	testLibs = map.get('libs', [])
	custom_target(
		test,
		command: [
			crunchMake, '-s', '@INPUT@', '-o', '@OUTPUT@', '-I@0@/src'.format(srcDir), '-I@0@/src'.format(buildDir),
			'-I@0@/src/@1@'.format(buildDir, host_machine.system())
		] + commandExtra + testLibs,
		input: [test + '.cxx'] + pcatObjs + testObjs,
		output: test + '.so',
		build_by_default: true
	)

	test(
		test,
		crunchpp,
		args: [test],
		workdir: meson.current_build_dir(),
		is_parallel: false
	)
endforeach

subdir('algorithm/blockLinear')
subdir('algorithm/chunkSpans')
