import Realm from 'realm';

{
	// Not providing anything
	const realm = new Realm();
}

{
	// Providing a path as a string
	const realm = new Realm('');
}

{
	// Providing a config object with a string
	const realm = new Realm({
		path: 'default.realm',
	});
}
